#ifndef __LOGIN_CONNECTION_H__
#define __LOGIN_CONNECTION_H__

#include "../src/tcp-connection.h"
#include "../src/inet-address.h"
#include "../src/singleton.h"
#include "../src/time-stamp.h"

#include <memory>
#include <string>
#include <unordered_map>

enum
{
    CONNECTION_HTTP_CLIENT = 1,
    CONNECTION_MESSAGE_SERVER = 2,
    CONNECTION_CLIENT = 3
};

namespace mg
{
    class EventLoop;
    class Buffer;
}

class ConnectionManger : public Singleton<ConnectionManger>
{
public:
    void addConnection(mg::TcpConnectionPointer &link);

    void removeConnection(const mg::TcpConnectionPointer &link);

private:
    std::unordered_map<std::string, mg::TcpConnectionPointer> _connections;
};

class HttpClientConnection : public mg::TcpConnection
{
public:
    HttpClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                         const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

private:
    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time);

    void connectionChangeCallback(const mg::TcpConnectionPointer &link);

    void writeCompleteCallback(const mg::TcpConnectionPointer &link);
};

class ClientConnection : public mg::TcpConnection
{
public:
    ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                     const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

private:
    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time);

    void connectionChangeCallback(const mg::TcpConnectionPointer &link);

    void writeCompleteCallback(const mg::TcpConnectionPointer &link);
};

class MessageServerConnection : public mg::TcpConnection
{
public:
    MessageServerConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                            const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

private:
    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time);

    void connectionChangeCallback(const mg::TcpConnectionPointer &link);

    void writeCompleteCallback(const mg::TcpConnectionPointer &link);
};

#endif //__LOGIN_CONNECTION_H__