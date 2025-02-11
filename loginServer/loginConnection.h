#ifndef __LOGIN_CONNECTION_H__
#define __LOGIN_CONNECTION_H__

#include "../src/base/tcp-connection.h"
#include "../src/base/inet-address.h"
#include "../src/base/singleton.h"
#include "../src/base/time-stamp.h"

#include <memory>
#include <string>
#include <unordered_map>

// 时间间隔单位都是微秒
#define SERVER_HEARTBEAT_INTERVAL 5000000
#define SERVER_TIMEOUT 30000000
#define CLIENT_HEARTBEAT_INTERVAL 30000000
#define CLIENT_TIMEOUT 120000000
#define MOBILE_CLIENT_TIMEOUT 60000000 * 5
#define HTTP_CONN_TIMEOUT 60000000

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
    void addConnection(mg::TcpConnectionPointer &link, int type);

    void removeConnection(const mg::TcpConnectionPointer &link);

    void Timer();

private:
    std::unordered_map<std::string, std::pair<mg::TcpConnectionPointer, int>> _connections;
};

class ConnectionBase : public mg::TcpConnection
{
public:
    ConnectionBase(mg::EventLoop *loop, const std::string &name, int sockfd,
                   const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

    inline const mg::TimeStamp &getLastReceiveTime() const { return this->_lastReceiveTime; };

    inline const mg::TimeStamp &getLastSendTime() const { return this->_lastSendTime; };

    inline void setLastSendTime(mg::TimeStamp curTime) { this->_lastSendTime = curTime; };

    inline void setLastReceiveTime(mg::TimeStamp curTime) { this->_lastReceiveTime = curTime; };

protected:
    virtual void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) {};

    virtual void connectionChangeCallback(const mg::TcpConnectionPointer &link) {};

    virtual void writeCompleteCallback(const mg::TcpConnectionPointer &link) {};

private:
    mg::TimeStamp _lastSendTime;
    mg::TimeStamp _lastReceiveTime;
};

class HttpClientConnection : public ConnectionBase
{
public:
    HttpClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                         const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

private:
    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) override;

    void connectionChangeCallback(const mg::TcpConnectionPointer &link) override;

    void writeCompleteCallback(const mg::TcpConnectionPointer &link) override;
};

class ClientConnection : public ConnectionBase
{
public:
    ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                     const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

private:
    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) override;

    void connectionChangeCallback(const mg::TcpConnectionPointer &link) override;

    void writeCompleteCallback(const mg::TcpConnectionPointer &link) override;
};

struct MessageServerInfo
{
    MessageServerInfo() : ip(), port(0), max_conn_cnt(0),
                          cur_conn_cnt(0), hostname()
    {
        ;
    }

    std::string ip;
    uint16_t port;
    uint32_t max_conn_cnt;
    uint32_t cur_conn_cnt;
    std::string hostname;
};

class MessageServerConnection : public ConnectionBase
{
public:
    MessageServerConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                            const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

private:
    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) override;

    void connectionChangeCallback(const mg::TcpConnectionPointer &link) override;

    void writeCompleteCallback(const mg::TcpConnectionPointer &link) override;

    void handleMessageServerInfo(const std::string &data);
};

#endif //__LOGIN_CONNECTION_H__