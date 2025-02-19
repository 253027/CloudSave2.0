#include "messageServer.h"
#include "../src/base/tcp-client.h"
#include "../src/base/json.hpp"
#include "../src/base/log.h"
#include "../src/base/connector.h"
#include "../src/base/inet-address.h"

#include <fstream>
#include <sstream>

using json = nlohmann::json;

MessageServer::MessageServer() : _maxConnection(0)
{
    ;
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

    std::string &&ip = config["listenIp"];
    uint16_t port = config["listenPort"];
    this->_maxConnection = config["maxConnection"];
    this->_server.reset(new mg::TcpServer(this->_loop.get(), mg::InternetAddress(ip, port), "MessageServer"));
    return true;
}

bool MessageServer::start()
{
    for (auto &client : _loginClientList)
        client->connect();
    this->_server->start();
    this->_loop->loop();
    return true;
}

void MessageServer::quit()
{
    this->_loop->quit();
    this->_server.reset();
}

std::string MessageServer::getIp()
{
    return this->_server->getIp();
}

uint16_t MessageServer::getPort()
{
    return this->_server->getPort();
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