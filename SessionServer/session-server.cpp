#include "session-server.h"

void SessionServer::onConnectionStateChanged(const mg::TcpConnectionPointer &connection)
{
    if (connection->connected())
        LOG_DEBUG("{} established", connection->name());
    else
        LOG_DEBUG("{} disconnected", connection->name());
}

void SessionServer::initial()
{
    _loop.reset(new mg::EventLoop("main-loop"));
    _server.reset(new mg::TcpServer(_loop.get(), mg::InternetAddress(9190), "SessionServer", mg::IPV4_DOMAIN, mg::TCP_SOCKET));
}

void SessionServer::start()
{
    _server->setConnectionCallback(std::bind(&SessionServer::onConnectionStateChanged, this, std::placeholders::_1));
    _server->setMessageCallback(std::bind(&SessionServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _server->setThreadNums(2);
    _server->start();
    _loop->loop();
}

void SessionServer::quit()
{
    _loop->quit();
    _server.reset();
}

void SessionServer::onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c)
{
    std::string data;
    if (!mg::TcpPacketParser::getMe().reveive(a, data))
        return;

    int type = 0;
    std::string userdata;
    std::tie(type, userdata) = Protocal::parse(data);

    switch (type)
    {
    case DataType::JSON_STRING:
        BusinessTask::getMe().parse(a, userdata);
        break;
    default:
        break;
    }
}
