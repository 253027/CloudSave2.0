#include "session-server-client.h"
#include "gateway-server.h"
#include "../ServerSDK/json.hpp"
#include "../ServerSDK/log.h"
#include "../ServerSDK/tcp-connection.h"
#include "../ServerSDK/tcp-client.h"
#include "../ServerSDK/eventloop-thread.h"
#include "../ServerSDK/tcp-packet-parser.h"

#include <fstream>

using json = nlohmann::json;

SessionClient::SessionClient()
{
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

void SessionClient::onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c)
{
    std::string data;
    if (!mg::TcpPacketParser::getMe().reveive(a, data))
        return;
    GateWayServer::getMe().onInternalServerResponse(a->name(), data);
}

void SessionClient::onConnectionStateChanged(const mg::TcpConnectionPointer &connection)
{
    if (connection->connected())
        LOG_INFO("{} connected to {}", connection->name(), connection->peerAddress().toIpPort());
    else
        LOG_INFO("{} disconnected from {}", connection->name(), connection->peerAddress().toIpPort());
}
