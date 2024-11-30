#include "session-server-client.h"
#include "gateway-server.h"
#include "../src/json.hpp"
#include "../src/log.h"
#include "../src/tcp-connection.h"
#include "../src/tcp-client.h"
#include "../src/eventloop-thread.h"
#include "../src/tcp-packet-parser.h"
#include "../src/log.h"

#include <fstream>

using json = nlohmann::json;

SessionClient::SessionClient() : _index(0)
{
    ;
}

SessionClient::~SessionClient()
{
    ;
}

bool SessionClient::initial()
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

    int clientnums = js["SessionClient"].value("clientnums", 1);
    std::string ip = js["SessionClient"].value("ip", "0.0.0.0");
    int16_t port = js["SessionClient"].value("port", 0);
    for (int i = 0; i < clientnums; i++)
    {
        std::stringstream threadname, clientname;
        threadname << "SessionClientThread#" << (i + 1);
        clientname << "SessionClient#" << (i + 1);

        _threads.emplace_back(std::make_unique<mg::EventLoopThread>(threadname.str()));
        mg::EventLoop *loop = _threads.back()->startLoop();

        _clients.emplace_back(std::make_unique<mg::TcpClient>(mg::IPV4_DOMAIN, mg::TCP_SOCKET, loop, mg::InternetAddress(ip, port), clientname.str()));
        _clients.back()->setMessageCallback(std::bind(&SessionClient::onMessage, this, std::placeholders::_1,
                                                      std::placeholders::_2, std::placeholders::_3));
        _clients.back()->setConnectionCallback(std::bind(&SessionClient::onConnectionStateChanged, this, std::placeholders::_1));
        _clients.back()->enableRetry();
        _clients.back()->connect();
    }

    return true;
}

bool SessionClient::sendToServer(const std::string &data)
{
    mg::TcpConnectionPointer p;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        if (_connections.empty())
            return false;
        p = _connections[_index].lock();
        _index = (_index + 1) % _connections.size();
    }
    if (!p)
        return false;
    mg::TcpPacketParser::getMe().send(p, data);
    return true;
}

void SessionClient::onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c)
{
    std::string data;
    if (!mg::TcpPacketParser::getMe().reveive(a, data))
        return;
    json js;
    try
    {
        js = json::parse(data);
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("{}", e.what());
        return;
    }

    if (!js.contains("connection-name") || !js["connection-name"].is_string())
    {
        LOG_ERROR("{} invalid connection-name type", a->name());
        return;
    }
    std::string name = js["connection-name"];
    js.erase("connection-name");

    GateWayServer::getMe().onInternalServerResponse(name, js.dump());
    LOG_DEBUG("{} data:\n{}", a->name(), data);
}

void SessionClient::onConnectionStateChanged(const mg::TcpConnectionPointer &connection)
{
    if (connection->connected())
    {
        {
            std::lock_guard<std::mutex> guard(_mutex);
            _connections.push_back(connection);
        }
        LOG_INFO("{} connected to {}", connection->name(), connection->peerAddress().toIpPort());
    }
    else
    {
        {
            std::lock_guard<std::mutex> guard(_mutex);
            for (int i = 0, j = _connections.size() - 1; i < j; i++)
            {
                if (_connections[i].lock() != connection)
                    continue;
                std::swap(_connections[i], _connections[j]);
                break;
            }
            _connections.pop_back();
        }
        LOG_INFO("{} disconnected from {}", connection->name(), connection->peerAddress().toIpPort());
    }
}
