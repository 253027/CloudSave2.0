#include "loginServerClient.h"
#include "messageServer.h"
#include "../src/base/tcp-client.h"
#include "../src/base/tcp-connection.h"
#include "../src/base/log.h"
#include "../src/base/tcp-packet-parser.h"
#include "../src/protocal/IM.Server.pb.h"
#include "../src/protocal/imPduBase.h"
#include "../src/protocal/IM.BaseDefine.pb.h"
#include "../src/protocal/IM.Other.pb.h"
#include "../src/common/common.h"

LoginServerClient::LoginServerClient(int domain, int type, mg::EventLoop *loop,
                                     const mg::InternetAddress &address, const std::string &name)
    : _client(new mg::TcpClient(domain, type, loop, address, name))
{
    _client->setConnectionCallback(std::bind(&LoginServerClient::connectionChangeCallback, this, std::placeholders::_1, loop));
    _client->setMessageCallback(std::bind(&LoginServerClient::messageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _client->enableRetry();
}

void LoginServerClient::connect()
{
    this->_client->connect();
}

void LoginServerClient::connectionChangeCallback(const mg::TcpConnectionPointer &link, mg::EventLoop *loop)
{
    if (link->connected())
    {
        // this is a test message
        IM::Server::IMMsgServInfo message;
        char hostname[256] = {0};
        ::gethostname(hostname, 256);
        message.set_ip(MessageServer::get().getIp());
        message.set_port(MessageServer::get().getPort());
        message.set_max_conn_cnt(MessageServer::get().getMaxConnection());
        message.set_cur_conn_cnt(0); // FIXME: 需要发送真实用户数
        message.set_host_name(hostname);

        PduMessage pdu;
        pdu.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);
        pdu.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_MSG_SERV_INFO);
        pdu.setPBMessage(&message);
        this->send(link, pdu.dump());

        this->setNextReceiveTime(mg::TimeStamp(mg::TimeStamp::now().getMircoSecond() + SERVER_HEARTBEAT_INTERVAL));
        loop->runEvery(SERVER_HEARTBEAT_INTERVAL / 1000000, std::bind(&LoginServerClient::heartBeatMessage, this, link));
        LOG_INFO("{} success connected to {}", link->name(), link->peerAddress().toIpPort());
    }
    else
    {
        LOG_INFO("{} disconnected from {}", link->name(), link->peerAddress().toIpPort());
    }
}

void LoginServerClient::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
{
    while (1)
    {
        std::string data;
        if (!mg::TcpPacketParser::get().reveive(link, data))
            break;

        std::unique_ptr<PduMessage> message(new PduMessage());
        if (!message->parse(data))
        {
            LOG_ERROR("{} wrong message", link->name());
            continue;
        }

        switch (message->getCommandId())
        {
        case IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT:
        {
            this->setNextReceiveTime(mg::TimeStamp(time.getMircoSecond() + SERVER_HEARTBEAT_INTERVAL));
            LOG_DEBUG("{} heart beat message", link->name());
            break;
        }
        }
    }
}

bool LoginServerClient::connected()
{
    return this->_client->connected();
}

mg::TcpConnectionPointer LoginServerClient::connection()
{
    return this->_client->connection();
}
