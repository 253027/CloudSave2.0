#include "loginConnection.h"
#include "../src/base/event-loop.h"
#include "../src/base/log.h"

void ConnectionManger::addConnection(mg::TcpConnectionPointer &link, int type)
{
    this->_connections[link->name()] = std::make_pair(link, type);
}

void ConnectionManger::removeConnection(const mg::TcpConnectionPointer &link)
{
    this->_connections.erase(link->name());
    mg::EventLoop *loop = link->getLoop();
    loop->push(std::bind(&mg::TcpConnection::connectionDestoryed, std::move(link)));
}

void ConnectionManger::Timer()
{
    mg::TimeStamp curTime = mg::TimeStamp::now();
    std::unordered_map<std::string, std::pair<mg::TcpConnectionPointer, int>> timeout;
    for (auto &x : this->_connections)
    {
        const auto link = std::dynamic_pointer_cast<ConnectionBase>(x.second.first);
        const auto type = x.second.second;
        switch (type)
        {
        case CONNECTION_HTTP_CLIENT:
        {
            if (curTime > mg::TimeStamp(link->getLastReceiveTime().getMircoSecond() + HTTP_CONN_TIMEOUT))
            {
                timeout[x.first] = x.second;
                LOG_DEBUG("{} httpClient Timeout", link->name());
            }
            break;
        }
        case CONNECTION_MESSAGE_SERVER:
        {
            if (curTime > mg::TimeStamp(link->getLastReceiveTime().getMircoSecond() + SERVER_TIMEOUT))
            {
                timeout[x.first] = x.second;
                LOG_DEBUG("{} messageServer Timeout", link->name());
            }

            if (curTime > mg::TimeStamp(link->getLastSendTime().getMircoSecond() + SERVER_HEARTBEAT_INTERVAL))
            {
                // TODO: send heart beat data
            }

            break;
        }
        case CONNECTION_CLIENT:
        {
            if (curTime > mg::TimeStamp(link->getLastReceiveTime().getMircoSecond() + CLIENT_TIMEOUT))
            {
                timeout[x.first] = x.second;
                LOG_DEBUG("{} messageServer Timeout", link->name());
            }
            break;
        }
        }
    }

    for (auto &x : timeout)
        x.second.first->forceClose();
}

ConnectionBase::ConnectionBase(mg::EventLoop *loop, const std::string &name, int sockfd,
                               const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : mg::TcpConnection(loop, name, sockfd, localAddress, peerAddress), _lastReceiveTime(mg::TimeStamp::now()),
      _lastSendTime(mg::TimeStamp::now())
{
    ;
}

HttpClientConnection::HttpClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                           const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : ConnectionBase(loop, name, sockfd, localAddress, peerAddress)
{
    this->setConnectionCallback(std::bind(&HttpClientConnection::connectionChangeCallback, this, std::placeholders::_1));
    this->setMessageCallback(std::bind(&HttpClientConnection::messageCallback, this,
                                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpClientConnection::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
{
    ;
}

void HttpClientConnection::connectionChangeCallback(const mg::TcpConnectionPointer &link)
{
    if (!link->connected())
    {
        LOG_DEBUG("{} disconnected", link->name());
    }
}

void HttpClientConnection::writeCompleteCallback(const mg::TcpConnectionPointer &link)
{
    ;
}

ClientConnection::ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                   const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : ConnectionBase(loop, name, sockfd, localAddress, peerAddress)
{
    this->setConnectionCallback(std::bind(&ClientConnection::connectionChangeCallback, this, std::placeholders::_1));
    this->setMessageCallback(std::bind(&ClientConnection::messageCallback, this,
                                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void ClientConnection::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
{
    ;
}

void ClientConnection::connectionChangeCallback(const mg::TcpConnectionPointer &link)
{
    if (!link->connected())
    {
        LOG_DEBUG("{} disconnected", link->name());
    }
}

void ClientConnection::writeCompleteCallback(const mg::TcpConnectionPointer &link)
{
    ;
}

MessageServerConnection::MessageServerConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                                 const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : ConnectionBase(loop, name, sockfd, localAddress, peerAddress)
{
    this->setConnectionCallback(std::bind(&MessageServerConnection::connectionChangeCallback, this, std::placeholders::_1));
    this->setMessageCallback(std::bind(&MessageServerConnection::messageCallback, this,
                                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void MessageServerConnection::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
{
    ;
}

void MessageServerConnection::connectionChangeCallback(const mg::TcpConnectionPointer &link)
{
    if (!link->connected())
    {
        LOG_DEBUG("{} disconnected", link->name());
    }
}

void MessageServerConnection::writeCompleteCallback(const mg::TcpConnectionPointer &link)
{
    ;
}
