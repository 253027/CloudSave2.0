#include "clientConnection.h"
#include "messageServer.h"
#include "messageUser.h"
#include "proxyServerClient.h"
#include "../src/base/log.h"
#include "../src/common/common-macro.h"

#include <atomic>

static std::atomic<uint32_t> up_message_total_count = {0};        // 上行消息总数
static std::atomic<uint32_t> up_message_miss_total_count = {0};   // 上行消息丢包数
static std::atomic<uint32_t> down_message_total_count = {0};      // 下行消息总数
static std::atomic<uint32_t> down_message_miss_total_count = {0}; // 下行消息丢包数

ClientConnection::ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                   const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : ConnectionBase(), TcpConnection(loop, name, sockfd, localAddress, peerAddress),
      _loginName(), _userId(0), _clientType(0), _isValid(false), _sendPerSeconds(0)
{
    this->setConnectionCallback(std::bind(&ClientConnection::connectionChangeCallback, this, std::placeholders::_1));
    this->setWriteCompleteCallback(std::bind(&ClientConnection::writeCompleteCallback, this, std::placeholders::_1));
    this->setMessageCallback(std::bind(&ClientConnection::messageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void ClientConnection::connectionChangeCallback(const mg::TcpConnectionPointer &link)
{
    if (link->connected())
    {
        ClientConnectionManger::get().addConnection(link->name(), link);
        link->runEvery(1.0, [this]()
                       { this->_sendPerSeconds = 0; });
        link->runEvery(1.0, std::bind(&ClientConnection::handleTimeoutMessage, this));
        LOG_INFO("new client connection:{}", link->name());
    }
    else
    {
        std::string name = link->name();
        this->updateUserStatus(IM::BaseDefine::USER_STATUS_OFFLINE);

        auto userByName = MessageUserManger::get().getUserByUserName(this->getLoginName());
        if (userByName)
        {
            userByName->removeUnvalidConnection(name);
            if (userByName->getUnvalidConnectionCount() == 0)
                MessageUserManger::get().removeUserByUserName(this->getLoginName());
        }

        auto userById = MessageUserManger::get().getUserByUserId(this->getUserId());
        if (userById)
        {
            userById->removeValidConnection(name);
            if (userById->getValidConnectionCount() == 0)
                MessageUserManger::get().removeUserByUserId(this->getUserId());
        }

        LOG_INFO("{} disconnected", link->name());
    }
}

void ClientConnection::writeCompleteCallback(const mg::TcpConnectionPointer &link)
{
    LOG_TRACE("{} write complete", link->name());
    if (link->getUserConnectionState())
    {
        link->forceClose();
        LOG_INFO("{} wirte complete and force close", link->name());
    }
}

void ClientConnection::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
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

        if (message->getCommandId() != IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_REQ && !this->isValid())
        {
            LOG_ERROR("{} unvalid message", link->name());
            continue;
        }

        switch (message->getCommandId())
        {
        case IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT:
            this->setNextReceiveTime(mg::TimeStamp(time.getMircoSecond() + SERVER_TIMEOUT));
            LOG_TRACE("{} heart beat message", link->name());
            break;
        case IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_REQ:
            this->handleLoginRequest(std::move(message));
            break;
        case IM::BaseDefine::COMMAND_MESSAGE_DATA:
            this->handleSendMessage(std::move(message));
            break;
        case IM::BaseDefine::COMMAND_ID_FRIEND_LIST_FRIEND_REQ:
            this->handleGetLatestFriendList(std::move(message));
            break;
        case IM::BaseDefine::COMMAND_MESSAGE_DATA_ACK:
            this->handleSendMessageAck(std::move(message));
            break;
        case IM::BaseDefine::COMMAND_TIME_REQUEST:
            this->handleGetTimeRequest(std::move(message));
            break;
        case IM::BaseDefine::COMMAND_MESSAGE_UNREAD_REQ:
            this->handleGetUnReadMessage(std::move(message));
            break;
        }
    }
}

void ClientConnection::send(const std::string &data)
{
    this->ConnectionBase::send(shared_from_this(), data);
}

void ClientConnection::addToSendList(uint32_t message_id, uint32_t other)
{
    down_message_total_count++;
    this->_send_list.emplace_back(message_id, other, mg::TimeStamp::now().getSeconds());
}

void ClientConnection::removeFromSendList(uint32_t message_id, uint32_t other)
{
    for (auto it = this->_send_list.begin(); it != this->_send_list.end(); it++)
    {
        uint32_t a, b;
        std::tie(a, b, std::ignore) = *it;
        if (a == message_id && b == other)
        {
            this->_send_list.erase(it);
            break;
        }
    }
}

void ClientConnection::updateUserStatus(uint32_t status)
{
    auto user = MessageUserManger::get().getUserByUserId(this->getUserId());
    if (!user) // unvalid connection
        return;
    if (status != IM::BaseDefine::USER_STATUS_ONLINE && status != IM::BaseDefine::USER_STATUS_OFFLINE)
        return;

    PduMessage pdu;
    pdu.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);
    pdu.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_USER_CNT_UPDATE);

    IM::Server::IMMsgServInfoUpdate message;
    message.set_user_id(this->getUserId());

    switch (status)
    {
    case IM::BaseDefine::USER_STATUS_ONLINE:
    {
        message.set_user_action(1);
        break;
    }
    case IM::BaseDefine::USER_STATUS_OFFLINE:
    {
        message.set_user_action(0);
        break;
    }
    }

    pdu.setPBMessage(&message);
    MessageServer::get().boardcastLoginServer(pdu.dump());
}

void ClientConnection::handleLoginRequest(std::unique_ptr<PduMessage> data)
{
    if (!this->_loginName.empty())
    {
        LOG_DEBUG("{} duplicate login request in same connection", this->name());
        return;
    }

    PARSE_PROTOBUF_MESSAGE(IM::Login::LoginRequest, request,
                           data->getPBmessage().retrieveAllAsString());

    uint32_t result = 0;
    std::string resultString = "Server abnormality";

    auto connection = ProxyServerClientManger::get().getHandle();
    if (!connection || !connection->connected())
        result = IM::BaseDefine::REFUSE_REASON_NO_PROXY_SERVER;
    else if (!MessageServer::get().loginServerAvaiable())
        result = IM::BaseDefine::REFUSE_REASON_NO_LOGIN_SERVER;

    if (result) // error occur
    {
        PduMessage message;
        IM::Login::LoginResponse response;
        response.set_server_time(mg::TimeStamp::now().getSeconds());
        response.set_refuse_type(static_cast<IM::BaseDefine::RefuseType>(result));
        response.set_result_string(resultString);
        message.setServiceId(IM::BaseDefine::SERVER_ID_LOGIN);
        message.setCommandId(IM::BaseDefine::COMMAND_LOGIN_RES_USER_LOGIN);
        message.setPBMessage(&response);
        this->send(message.dump());
        return;
    }

    this->_loginName = request.user_name();
    std::string password = request.password();
    uint32_t onlineStatus = request.status_type();
    this->_clientType = request.client_type();
    if (onlineStatus < IM::BaseDefine::USER_STATUS_ONLINE || onlineStatus > IM::BaseDefine::USER_STATUS_LEAVE)
    {
        LOG_INFO("{} online status error", this->name());
        onlineStatus = IM::BaseDefine::USER_STATUS_ONLINE;
    }

    auto user = MessageUserManger::get().getUserByUserName(request.user_name());
    if (!user)
    {
        user = std::make_shared<MessageUser>(request.user_name());
        MessageUserManger::get().addUserByUserName(request.user_name(), user);
    }
    user->addUnValidConnection(this->name(), std::dynamic_pointer_cast<ClientConnection>(shared_from_this()));

    PduMessage messagePdu;
    messagePdu.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);
    messagePdu.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_REQ);
    IM::Server::VerifyDataRequest proxyRequest;
    proxyRequest.set_user_name(request.user_name());
    proxyRequest.set_password(request.password());
    proxyRequest.set_attach_data(this->name());
    messagePdu.setPBMessage(&proxyRequest);
    connection->send(messagePdu.dump()); // send to proxy server valid this user
}

void ClientConnection::handleGetLatestFriendList(std::unique_ptr<PduMessage> data)
{
    PARSE_PROTOBUF_MESSAGE(IM::Buddy::IMGetFriendListRequest, request,
                           data->getPBmessage().retrieveAllAsString());
    uint32_t result = 0;
    std::string resultString = "Server abnormality";
    auto connection = ProxyServerClientManger::get().getHandle();
    if (!connection || !connection->connected())
    {
        result = IM::BaseDefine::REFUSE_REASON_NO_PROXY_SERVER;
        return;
    }

    PduMessage pdu;
    pdu.setServiceId(IM::BaseDefine::SERVER_ID_BUDDY_LIST);
    pdu.setCommandId(IM::BaseDefine::COMMAND_ID_FRIEND_LIST_FRIEND_REQ);
    request.set_attach_data(this->name());
    pdu.setPBMessage(&request);
    connection->send(pdu.dump());
}

void ClientConnection::handleSendMessage(std::unique_ptr<PduMessage> data)
{
    PARSE_PROTOBUF_MESSAGE(IM::Message::MessageData, message,
                           data->getPBmessage().retrieveAllAsString());
    if (message.message_data().empty())
        return;
    if (++this->_sendPerSeconds > MAX_SEND_MESSAGE_PERSECOND)
    {
        LOG_INFO("{} send to much message per second {}", this->getLoginName(), this->_sendPerSeconds);
        return;
    }

    auto proxyClient = ProxyServerClientManger::get().getHandle();
    if (!proxyClient || !proxyClient->connected())
    {
        LOG_WARN("proxy server is not connected");
        return;
    }

    PduMessage pdu;
    pdu.setServiceId(IM::BaseDefine::SERVER_ID_MESSAGE);
    pdu.setCommandId(IM::BaseDefine::COMMAND_MESSAGE_DATA);
    message.set_from(this->getUserId());
    message.set_create_time(mg::TimeStamp::now().getSeconds());
    message.set_attach_data(this->name());
    pdu.setPBMessage(&message);
    proxyClient->send(pdu.dump());
}

void ClientConnection::handleTimeoutMessage()
{
    uint32_t curTime = mg::TimeStamp::now().getSeconds();
    for (auto it = this->_send_list.begin(); it != this->_send_list.end();)
    {
        uint32_t message_id, other, time;
        std::tie(message_id, other, time) = *it;
        if (curTime < time + TIMEOUT_WAITING_MSG_DATA_ACK)
            break;
        it = this->_send_list.erase(it);
        down_message_miss_total_count++;
        LOG_ERROR("message {} between {} and {} missed", message_id, other, this->getUserId());
    }
}

void ClientConnection::handleSendMessageAck(std::unique_ptr<PduMessage> data)
{
    {
        auto proxyClient = ProxyServerClientManger::get().getHandle();
        if (proxyClient)
            proxyClient->send(data->dump());
    }

    PARSE_PROTOBUF_MESSAGE(IM::Message::MessageDataAck, message,
                           data->getPBmessage().retrieveAllAsString());
    uint32_t other = message.to() == this->getUserId() ? message.from() : message.to();
    this->removeFromSendList(message.message_id(), other);
}

void ClientConnection::handleGetTimeRequest(std::unique_ptr<PduMessage> data)
{
    PARSE_PROTOBUF_MESSAGE(IM::Message::TimeRequest, message,
                           data->getPBmessage().retrieveAllAsString());
    IM::Message::TimeResponse response;
    response.set_time_stamp(mg::TimeStamp::now().getSeconds());
    PduMessage pdu;
    pdu.setServiceId(IM::BaseDefine::SERVER_ID_MESSAGE);
    pdu.setCommandId(IM::BaseDefine::COMMAND_TIME_RESPONSE);
    pdu.setPBMessage(&response);
    this->send(pdu.dump());
}

void ClientConnection::handleGetUnReadMessage(std::unique_ptr<PduMessage> data)
{
    PARSE_PROTOBUF_MESSAGE(IM::Message::MessageUnReadRequest, message,
                           data->getPBmessage().retrieveAllAsString());

    auto proxyClient = ProxyServerClientManger::get().getHandle();
    if (!proxyClient)
        return;

    message.set_user_id(this->getUserId());
    message.set_attach_data(this->name());
    data->setPBMessage(&message);
    proxyClient->send(data->dump());
}

void ClientConnectionManger::addConnection(const std::string &name, const mg::TcpConnectionPointer &connection)
{
    this->_memo[name] = connection;
}

void ClientConnectionManger::removeConnection(const std::string &name)
{
    this->_memo.erase(name);
}

std::shared_ptr<ClientConnection> ClientConnectionManger::getConnctionByName(const std::string &name)
{
    auto it = this->_memo.find(name);
    if (it == this->_memo.end())
        return std::shared_ptr<ClientConnection>();
    return std::dynamic_pointer_cast<ClientConnection>(it->second);
}