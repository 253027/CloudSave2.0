#include "gateway-server.h"
#include "json-data-parser.h"
#include "session-server-client.h"
#include "../src/json.hpp"
#include "../src/tcp-server.h"
#include "../src/event-loop.h"
#include "../src/http-packet-parser.h"
#include "../src/tcp-packet-parser.h"
#include "../src/log.h"

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

void GateWayServer::onInternalServerResponse(const std::string &name, nlohmann::json &js)
{
    mg::TcpConnectionPointer p;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        auto it = _connection.find(name);
        if (it == _connection.end())
            return;
        p = it->second.lock();
        if (!p)
        {
            _connection.erase(name);
            return;
        }
        if(js.value("con-state", 0))
            p->setUserConnectionState(js["con-state"]);
        js.erase("con-state");
    }

    mg::HttpData httpData;
    mg::HttpHead &head = std::get<0>(httpData);
    mg::HttpBody &body = std::get<1>(httpData);
    head["HTTP/1.1"] = "200 OK";
    head["Content-Type"] = "application/json";
    head["Server"] = "Apache/2.4.41 (Ubuntu)";
    // body = "<html>Hello World</html>";
    body = js.dump();
    mg::HttpPacketParser::getMe().send(p, httpData);
}

void GateWayServer::onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c)
{
    mg::HttpData data;
    if (!mg::HttpPacketParser::getMe().reveive(a, data))
        return;

    mg::HttpHead head;
    mg::HttpBody body;
    std::tie(head, body) = std::move(data);
    bool valid = true;

    int type = mg::HttpPacketParser::getMe().parseType(head["content-type"]);
    switch (type)
    {
    case 7: // json数据
    {
        valid = JsonDataParser::getMe().parse(a, body);
        break;
    }
    default:
        valid = false;
        break;
    }

    if (!valid)
        this->invalidResponse(a);
}

void GateWayServer::connectionStateChange(const mg::TcpConnectionPointer &a)
{
    if (a->connected())
    {
        {
            std::lock_guard<std::mutex> guard(_mutex);
            _connection[a->name()] = a;
        }
        LOG_INFO("{} connected", a->peerAddress().toIpPort());
    }
    else
    {
        {
            std::lock_guard<std::mutex> guard(_mutex);
            _connection.erase(a->name());
        }
        LOG_INFO("{} disconnected", a->peerAddress().toIpPort());
    }
}

void GateWayServer::invalidResponse(const mg::TcpConnectionPointer &a)
{
    mg::HttpData httpData;
    mg::HttpHead &head = std::get<0>(httpData);
    mg::HttpBody &body = std::get<1>(httpData);
    head["HTTP/1.1"] = "400 Bad Request";
    head["Content-Type"] = "text/html";
    body = "<html>Invalid Request</html>";
    mg::HttpPacketParser::getMe().send(a, httpData);
}