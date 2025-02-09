#include "loginServer.h"
#include "loginConnection.h"
#include "../src/log.h"
#include "../src/json.hpp"
#include "../src/event-loop.h"
#include "../src/json-extract.h"
#include "../src/log.h"

#include <fstream>
#include <sstream>

using json = nlohmann::json;

bool LoginServer::initial(const std::string &configPath)
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

    std::string ip;
    uint16_t port;

    this->_loop.reset(new mg::EventLoop("loginServer"));

    // messageServer
    {
        ip = config["MessageServer"]["ip"];
        port = config["MessageServer"]["port"];
        this->_messageServer.reset(new mg::Acceptor(mg::IPV4_DOMAIN, mg::TCP_SOCKET, this->_loop.get(), mg::InternetAddress(ip, port), true));
        this->_messageServer->setNewConnectionCallBack(std::bind(&LoginServer::acceptorCallback, this,
                                                                 std::placeholders::_1, std::placeholders::_2, CONNECTION_MESSAGE_SERVER));
    }

    // httpClient
    {
        ip = config["HttpClient"]["ip"];
        port = config["HttpClient"]["port"];
        this->_httpClient.reset(new mg::Acceptor(mg::IPV4_DOMAIN, mg::TCP_SOCKET, this->_loop.get(), mg::InternetAddress(ip, port), true));
        this->_httpClient->setNewConnectionCallBack(std::bind(&LoginServer::acceptorCallback, this,
                                                              std::placeholders::_1, std::placeholders::_2, CONNECTION_HTTP_CLIENT));
    }

    // client
    {
        ip = config["Client"]["ip"];
        port = config["Client"]["port"];
        this->_client.reset(new mg::Acceptor(mg::IPV4_DOMAIN, mg::TCP_SOCKET, this->_loop.get(), mg::InternetAddress(ip, port), true));
        this->_client->setNewConnectionCallBack(std::bind(&LoginServer::acceptorCallback, this,
                                                          std::placeholders::_1, std::placeholders::_2, CONNECTION_CLIENT));
    }

    return true;
}

bool LoginServer::start()
{
    if (this->_start)
        return false;
    this->_start = true;

    this->_loop->run(std::bind(&mg::Acceptor::listen, this->_messageServer.get()));
    this->_loop->run(std::bind(&mg::Acceptor::listen, this->_client.get()));
    this->_loop->run(std::bind(&mg::Acceptor::listen, this->_httpClient.get()));
    this->_loop->runEvery(1.0, std::bind(&ConnectionManger::Timer, ConnectionManger::getInstance())); // seconds Timer
    this->_loop->loop();
    return true;
}

void LoginServer::quit()
{
    ;
}

void LoginServer::acceptorCallback(int fd, const mg::InternetAddress &peerAddress, int state)
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
    mg::TcpConnectionPointer connect;

    switch (state)
    {
    case CONNECTION_HTTP_CLIENT:
    {
        connect = std::make_shared<HttpClientConnection>(loop, peerAddress.toIpPort(), fd, localAddress, peerAddress);
        break;
    }
    case CONNECTION_MESSAGE_SERVER:
    {
        connect = std::make_shared<MessageServerConnection>(loop, peerAddress.toIpPort(), fd, localAddress, peerAddress);
        break;
    }
    case CONNECTION_CLIENT:
    {
        connect = std::make_shared<ClientConnection>(loop, peerAddress.toIpPort(), fd, localAddress, peerAddress);
        break;
    }
    default:
        LOG_ERROR("invalid connnection type");
        return;
    }

    ConnectionManger::get().addConnection(connect, state);
    connect->setCloseCallback(std::bind(&ConnectionManger::removeConnection, ConnectionManger::getInstance(), std::placeholders::_1));
    loop->run(std::bind(&mg::TcpConnection::connectionEstablished, connect.get()));
    LOG_INFO("new connection:[{}] socketFd:[{}]", peerAddress.toIpPort(), fd);
}
