#include "gateway-server.h"
#include "../ServerSDK/json.hpp"
#include "../ServerSDK/tcp-server.h"
#include "../ServerSDK/event-loop.h"
#include "../ServerSDK/http-packet-parser.h"
#include "../ServerSDK/tcp-packet-parser.h"
#include "json-data-parser.h"
#include "session-server-client.h"

#include <fstream>

using json = nlohmann::json;

GateWayServer::GateWayServer()
{
    mg::HttpPacketParser::getMe();
}

GateWayServer::~GateWayServer()
{
    mg::HttpPacketParser::destroyInstance();
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
    _server.reset(new mg::TcpServer(_loop.get(), mg::InternetAddress(js["GatewayServer"].value("port", 10800)), "GatewayServer"));
    _server->setMessageCallback(std::bind(&GateWayServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _server->setConnectionCallback(std::bind(&GateWayServer::connectionStateChange, this, std::placeholders::_1));
    _server->setThreadNums(js["GatewayServer"].value("threadnums", 1));
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

void GateWayServer::onInternalServerResponse(const std::string &name, std::string &data)
{
    auto it = _connection.find(name);
    if (it == _connection.end())
        return;
    mg::TcpConnectionPointer p = it->second.lock();
    if (!p)
    {
        _connection.erase(it);
        return;
    }
    // Todo: http报文生成
    mg::TcpPacketParser::getMe().send(p, data);
}

void GateWayServer::onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c)
{
    mg::HttpData data;
    if (!mg::HttpPacketParser::getMe().reveive(a, data))
        return;

    mg::HttpHead head;
    mg::HttpBody body;
    std::tie(head, body) = std::move(data);

    int type = mg::HttpPacketParser::getMe().parseType(head["content-type"]);
    switch (type)
    {
    case 7: // json数据
    {
        JsonDataParser::getMe().parse(a->name(), body);
        break;
    }
    }
}

void GateWayServer::connectionStateChange(const mg::TcpConnectionPointer &a)
{
    ;
}
