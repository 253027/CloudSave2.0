#include "gateway-server.h"
#include "../ServerSDK/json.hpp"
#include "../ServerSDK/tcp-server.h"
#include "../ServerSDK/event-loop.h"

#include <fstream>

using json = nlohmann::json;

GateWayServer::GateWayServer()
{
    ;
}

bool GateWayServer::initial()
{
    json js;
    std::ifstream file("./GatewayServer/config.json");
    if (!file.is_open())
    {
        LOG_ERROR("open config.json failed");
        return false;
    }

    try
    {
        file >> js;
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("config.json parse error: {}", e.what());
        return false;
    }

    _loop.reset(new mg::EventLoop("gateway-loop"));
    _server.reset(new mg::TcpServer(_loop.get(), mg::InternetAddress(js.value("port", 10800)), "GatewayServer"));
    _server->setMessageCallback(std::bind(&GateWayServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _server->setConnectionCallback(std::bind(&GateWayServer::connectionStateChange, this, std::placeholders::_1));
    _server->setThreadNums(js.value("threadnums", 1));
    return true;
}

void GateWayServer::start()
{
    _server->start();
    _loop->loop();
}

void GateWayServer::quit()
{
    _server.reset();
    _loop->quit();
}

void GateWayServer::onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c)
{
    ;
}

void GateWayServer::connectionStateChange(const mg::TcpConnectionPointer &a)
{
    ;
}
