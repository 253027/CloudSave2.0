#include "loginConnection.h"
#include "../src/base/event-loop.h"
#include "../src/base/log.h"
#include "../src/base/tcp-packet-parser.h"
#include "../src/protocal/IM.BaseDefine.pb.h"
#include "../src/protocal/IM.Server.pb.h"
#include "../src/protocal/imPduBase.h"
#include "../src/protocal/IM.Other.pb.h"

static std::unordered_map<std::string, std::unique_ptr<struct MessageServerInfo>> _messageServeList;
static uint32_t _total_online_user = 0;

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
        const auto type = x.second.second;
        switch (type)
        {
        case CONNECTION_HTTP_CLIENT:
        {
            const auto link = std::dynamic_pointer_cast<HttpClientConnection>(x.second.first);
            if (curTime > link->getNextReceiveTime())
            {
                timeout[x.first] = x.second;
                LOG_DEBUG("{} httpClient Timeout", link->name());
            }
            break;
        }
        case CONNECTION_MESSAGE_SERVER:
        {
            const auto link = std::dynamic_pointer_cast<MessageServerConnection>(x.second.first);
            if (curTime > link->getNextReceiveTime())
            {
                timeout[x.first] = x.second;
                LOG_DEBUG("{} messageServer Timeout", link->name());
            }

            if (curTime > link->getNextSendTime())
            {
                IM::Other::IMHeartBeat message;
                PduMessage pdu;
                pdu.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);
                pdu.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT);
                pdu.setPBMessage(&message);
                link->setNextSendTime(mg::TimeStamp(curTime.getMircoSecond() + SERVER_HEARTBEAT_INTERVAL));
                mg::TcpPacketParser::get().send(link, pdu.dump());
            }

            break;
        }
        case CONNECTION_CLIENT:
        {
            const auto link = std::dynamic_pointer_cast<ClientConnection>(x.second.first);
            if (curTime > link->getNextReceiveTime())
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
    : mg::TcpConnection(loop, name, sockfd, localAddress, peerAddress)
{
    this->setConnectionCallback(std::bind(&MessageServerConnection::connectionChangeCallback, this, std::placeholders::_1));
    this->setMessageCallback(std::bind(&MessageServerConnection::messageCallback, this,
                                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void MessageServerConnection::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
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
        case IM::BaseDefine::COMMAND_ID_OTHER_MSG_SERV_INFO:
        {
            auto pbMessage = message->getPBmessage();
            this->handleMessageServerInfo(pbMessage.retrieveAllAsString());
            break;
        }
        case IM::BaseDefine::COMMAND_ID_OTHER_USER_CNT_UPDATE:
        {
            break;
        }
        }
    }
}

void MessageServerConnection::connectionChangeCallback(const mg::TcpConnectionPointer &link)
{
    if (!link->connected())
    {
        auto it = _messageServeList.find(link->name());
        if (it != _messageServeList.end())
        {
            _total_online_user -= it->second->cur_conn_cnt;
            _messageServeList.erase(it);
        }
        LOG_DEBUG("{} disconnected", link->name());
    }
    else
        this->setNextReceiveTime(mg::TimeStamp(mg::TimeStamp::now().getMircoSecond() + SERVER_HEARTBEAT_INTERVAL));
}

void MessageServerConnection::writeCompleteCallback(const mg::TcpConnectionPointer &link)
{
    ;
}

void MessageServerConnection::handleMessageServerInfo(const std::string &data)
{
    IM::Server::IMMsgServInfo message;
    if (!message.ParseFromString(data))
    {
        LOG_ERROR("{} parse protobuf message error", this->name());
        return;
    }

    std::unique_ptr<MessageServerInfo> server(new MessageServerInfo());
    server->ip = std::move(message.ip());
    server->port = message.port();
    server->max_conn_cnt = message.max_conn_cnt();
    server->cur_conn_cnt = message.cur_conn_cnt();
    server->hostname = std::move(message.host_name());

    _total_online_user += server->cur_conn_cnt;
    LOG_INFO("MessageInfo ip[{}] port[{}] max_conn_cnt[{}] cur_con_cnt[{}] hostname[{}]",
             server->ip, server->port, server->max_conn_cnt, server->cur_conn_cnt, server->hostname);
    _messageServeList[this->name()] = std::move(server);
}
