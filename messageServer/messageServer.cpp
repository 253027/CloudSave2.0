#include "messageServer.h"
#include "clientConnection.h"
#include "../src/base/tcp-client.h"
#include "../src/base/json.hpp"
#include "../src/base/log.h"
#include "../src/base/connector.h"
#include "../src/base/inet-address.h"
#include "../src/base/acceptor.h"

#include <fstream>
#include <sstream>

using json = nlohmann::json;

MessageServer::MessageServer() : _ip(), _port(0), _maxConnection(0)
{
    ;
}

MessageServer::~MessageServer()
{
    // 析构前需要将清除定时器队列，以防止有未关闭的TcpConnnection连接还在执行定时任务
    for (auto &x : this->_timerMemo)
        this->_loop->cancel(x);
    for (auto &client : this->_loginClientList)
    {
        client->disableRetry();
        auto end = client->connection();
        if (end)
            end->forceClose();
    }
}

bool MessageServer::initial(const std::string &configPath)
{
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        LOG_ERROR("read configuration File failed");
        return false;
    }

    json config;
    try
    {
        file >> config;
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("configPath parse error: {}", e.what());
        return false;
    }

    this->_loop.reset(new mg::EventLoop("loginServer"));

    // loginServerClient
    {
        for (auto &server : config["LoginServer"])
        {
            std::string ip = server["ip"];
            uint16_t port = server["port"];
            _loginClientList.emplace_back(std::unique_ptr<LoginServerClient>(new LoginServerClient(mg::IPV4_DOMAIN, mg::TCP_SOCKET, _loop.get(),
                                                                                                   mg::InternetAddress(ip, port), ip)));
        }
    }

    this->_ip = config["listenIp"];
    this->_port = config["listenPort"];
    this->_maxConnection = config["maxConnection"];
    this->_acceptor.reset(new mg::Acceptor(mg::IPV4_DOMAIN, mg::TCP_SOCKET, this->_loop.get(), mg::InternetAddress(this->_ip, this->_port), true));
    this->_acceptor->setNewConnectionCallBack(std::bind(&MessageServer::acceptorCallback, this, std::placeholders::_1, std::placeholders::_2));
    this->_ip = config["ip"];
    return true;
}

bool MessageServer::start()
{
    for (auto &client : _loginClientList)
        client->connect();
    this->_acceptor->listen();
    this->_loop->loop();
    return true;
}

void MessageServer::quit()
{
    this->_loop->quit();
}

std::string MessageServer::getIp()
{
    return this->_ip;
}

uint16_t MessageServer::getPort()
{
    return this->_port;
}

uint16_t MessageServer::getMaxConnection()
{
    return this->_maxConnection;
}

bool MessageServer::loginServerAvaiable()
{
    for (auto &con : this->_loginClientList)
    {
        if (!con->connected())
            continue;
        return true;
    }
    return false;
}

void MessageServer::addTimerID(mg::TimerId id)
{
    this->_timerMemo.emplace_back(std::move(id));
}

void MessageServer::acceptorCallback(int fd, const mg::InternetAddress &peerAddress)
{
    mg::EventLoop *loop = this->_loop.get();
    sockaddr_in local;
    ::memset(&local, 0, sizeof(local));
    socklen_t addresslen = sizeof(local);
    if (::getsockname(fd, (sockaddr *)&local, &addresslen) < 0)
    {
        LOG_ERROR("get local address failed");
        return;
    }

    mg::InternetAddress localAddress(local);
    std::shared_ptr<ClientConnection> connect = std::make_shared<ClientConnection>(loop, peerAddress.toIpPort(), fd, localAddress, peerAddress);

    auto removeConnection = [](const mg::TcpConnectionPointer &link)
    {
        ClientConnectionManger::get().removeConnection(link->name());
        mg::EventLoop *loop = link->getLoop();
        loop->push(std::bind(&mg::TcpConnection::connectionDestoryed, std::move(link)));
    };

    ClientConnectionManger::get().addConnection(connect->name(), connect);
    connect->setCloseCallback(std::bind(removeConnection, std::placeholders::_1));
    loop->run(std::bind(&mg::TcpConnection::connectionEstablished, connect.get()));
    LOG_INFO("new connection:[{}] socketFd:[{}]", peerAddress.toIpPort(), fd);
}