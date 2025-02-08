#include "loginConnection.h"
#include "../src/event-loop.h"

void ConnectionManger::addConnection(mg::TcpConnectionPointer &link)
{
    this->_connections[link->name()] = link;
}

void ConnectionManger::removeConnection(const mg::TcpConnectionPointer &link)
{
    this->_connections.erase(link->name());
    mg::EventLoop *loop = link->getLoop();
    loop->push(std::bind(&mg::TcpConnection::connectionDestoryed, std::move(link)));
}

HttpClientConnection::HttpClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                           const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : mg::TcpConnection(loop, name, sockfd, localAddress, peerAddress)
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
    ;
}

ClientConnection::ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                   const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : mg::TcpConnection(loop, name, sockfd, localAddress, peerAddress)
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
    ;
}

MessageServerConnection::MessageServerConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                                 const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : mg::TcpConnection(loop, name, sockfd, localAddress, peerAddress)
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
    ;
}
