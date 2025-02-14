#ifndef __COMMON_H__
#define __COMMON_H__

// 时间间隔单位都是微秒
#define SERVER_HEARTBEAT_INTERVAL 5000000
#define SERVER_TIMEOUT 30000000
#define CLIENT_HEARTBEAT_INTERVAL 30000000
#define CLIENT_TIMEOUT 120000000
#define MOBILE_CLIENT_TIMEOUT 60000000 * 5
#define HTTP_CONN_TIMEOUT 60000000

#include "../base/tcp-connection.h"
#include "../base/inet-address.h"
#include "../base/time-stamp.h"

class ConnectionBase
{
public:
    ConnectionBase() : _lastReceiveTime(mg::TimeStamp::now()), _lastSendTime(mg::TimeStamp::now()) {}

    inline const mg::TimeStamp &getNextReceiveTime() const { return this->_lastReceiveTime; };

    inline const mg::TimeStamp &getNextSendTime() const { return this->_lastSendTime; };

    inline void setNextSendTime(mg::TimeStamp curTime) { this->_lastSendTime = curTime; };

    inline void setNextReceiveTime(mg::TimeStamp curTime) { this->_lastReceiveTime = curTime; };

protected:
    virtual void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) {};

    virtual void connectionChangeCallback(const mg::TcpConnectionPointer &link) {};

    virtual void writeCompleteCallback(const mg::TcpConnectionPointer &link) {};

private:
    mg::TimeStamp _lastSendTime;
    mg::TimeStamp _lastReceiveTime;
};

#endif //__COMMON_H__