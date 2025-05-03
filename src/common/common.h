#ifndef __COMMON_H__
#define __COMMON_H__

// 时间间隔单位都是微秒
#define SERVER_HEARTBEAT_INTERVAL 5000000
#define SERVER_TIMEOUT 30000000
#define CLIENT_HEARTBEAT_INTERVAL 30000000
#define CLIENT_TIMEOUT 120000000
#define MOBILE_CLIENT_TIMEOUT 60000000 * 5
#define HTTP_CONN_TIMEOUT 60000000

// 时间间隔单位都是秒
#define MAX_SEND_MESSAGE_PERSECOND 10
#define TIMEOUT_WAITING_MSG_DATA_ACK 15

#include "../base/tcp-connection.h"
#include "../base/inet-address.h"
#include "../base/time-stamp.h"
#include "../base/event-loop.h"
#include "../base/tcp-packet-parser.h"
#include "../base/log.h"
#include "../base/time-stamp.h"
#include "../protocal/IM.BaseDefine.pb.h"
#include "../protocal/IM.Login.pb.h"
#include "../protocal/IM.Server.pb.h"
#include "../protocal/imPduBase.h"
#include "../protocal/IM.Other.pb.h"
#include "../protocal/IM.Message.pb.h"
#include "../protocal/IM.Buddy.pb.h"
#include "common-macro.h"

class ConnectionBase
{
public:
    ConnectionBase() : _lastReceiveTime(mg::TimeStamp::now()), _lastSendTime(mg::TimeStamp::now()) {}

    inline const mg::TimeStamp &getNextReceiveTime() const { return this->_lastReceiveTime; };

    inline const mg::TimeStamp &getNextSendTime() const { return this->_lastSendTime; };

    inline void setNextSendTime(mg::TimeStamp curTime) { this->_lastSendTime = curTime; };

    inline void setNextReceiveTime(mg::TimeStamp curTime) { this->_lastReceiveTime = curTime; };

    inline void send(const mg::TcpConnectionPointer &link, const std::string &data)
    {
        this->setNextSendTime(mg::TimeStamp(mg::TimeStamp::now().getMircoSecond() + SERVER_HEARTBEAT_INTERVAL));
        mg::TcpPacketParser::get().send(link, std::move(data));
    }

    inline void heartBeatMessage(const mg::TcpConnectionPointer &link)
    {
        mg::TimeStamp curTime = mg::TimeStamp::now();

        if (curTime > this->getNextSendTime())
        {
            IM::Other::IMHeartBeat message;
            PduMessage pdu;
            pdu.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);
            pdu.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT);
            pdu.setPBMessage(&message);
            this->send(link, pdu.dump());
        }

        if (curTime > this->getNextReceiveTime())
        {
            link->forceClose();
        }
    }

protected:
    virtual void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) {};

    virtual void connectionChangeCallback(const mg::TcpConnectionPointer &link) {};

    virtual void writeCompleteCallback(const mg::TcpConnectionPointer &link) {};

private:
    mg::TimeStamp _lastSendTime;
    mg::TimeStamp _lastReceiveTime;
};

#endif //__COMMON_H__