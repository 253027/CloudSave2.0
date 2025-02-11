#include "loginConnection.h"
#include "../src/base/event-loop.h"
#include "../src/base/log.h"
#include "../src/base/tcp-packet-parser.h"
#include "../src/protocal/IM.BaseDefine.pb.h"
#include "../src/protocal/IM.Server.pb.h"
#include "../src/protocal/imPduBase.h"

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
                IM::BaseDefine::ServiceID::SERVER_ID_LOGIN;
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
        _messageServeList.erase(link->name());
        LOG_DEBUG("{} disconnected", link->name());
    }
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
