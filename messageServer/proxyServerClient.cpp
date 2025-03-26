#include "proxyServerClient.h"
#include "clientConnection.h"
#include "messageUser.h"
#include "../src/base/event-loop.h"
#include "../src/base/tcp-client.h"
#include "../src/base/log.h"
#include "../src/protocal/IM.Login.pb.h"

ProxyServerClient::ProxyServerClient(int domain, int type, mg::EventLoop *loop,
                                     const mg::InternetAddress &address, const std::string &name)
    : _client(new mg::TcpClient(domain, type, loop, address, name))
{
    _client->setConnectionCallback(std::bind(&ProxyServerClient::connectionChangeCallback, this, std::placeholders::_1, loop));
    _client->setMessageCallback(std::bind(&ProxyServerClient::messageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _client->enableRetry();
}

bool ProxyServerClient::connected()
{
    return this->_client->connected();
}

mg::TcpConnectionPointer ProxyServerClient::connection()
{
    return this->_client->connection();
}

void ProxyServerClient::send(const std::string &data)
{
    this->ConnectionBase::send(this->_client->connection(), data);
}

void ProxyServerClient::connectionChangeCallback(const mg::TcpConnectionPointer &link, mg::EventLoop *loop)
{
    if (link->connected())
    {
        ProxyServerClientManger::get().addConnection(this);
        this->setNextReceiveTime(mg::TimeStamp(mg::TimeStamp::now().getMircoSecond() + SERVER_TIMEOUT));
        link->runEvery(SERVER_HEARTBEAT_INTERVAL / 1000000, std::bind(&ProxyServerClient::heartBeatMessage, this, link));
        LOG_INFO("{} success connected to {}", link->name(), link->peerAddress().toIpPort());
    }
    else
    {
        ProxyServerClientManger::get().removeConnection(this);
        LOG_INFO("{} disconnected from {}", link->name(), link->peerAddress().toIpPort());
    }
}

void ProxyServerClient::connect()
{
    this->_client->connect();
}

void ProxyServerClient::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
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

        data = message->getPBmessage().retrieveAllAsString();
        switch (message->getCommandId())
        {
        case IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT:
        {
            this->setNextReceiveTime(mg::TimeStamp(time.getMircoSecond() + SERVER_TIMEOUT));
            LOG_TRACE("{} heart beat message", link->name());
            break;
        }
        case IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_RSP:
        {
            this->_handleVerifyDataResponse(data);
            break;
        }
        case IM::BaseDefine::COMMAND_MESSAGE_DATA:
        {
            this->_handleSendMessageResponse(data);
            break;
        }
        }
    }
}

void ProxyServerClient::_handleVerifyDataResponse(const std::string &data)
{
    IM::Server::VerifyDataResponse message;
    if (!message.ParseFromString(data))
    {
        LOG_ERROR("{} parse protobuf message error", this->_client->connection()->name());
        return;
    }

    std::string loginName = message.user_name();
    uint32_t result = message.result_code();
    std::string resultString = message.result_string();

    auto userByName = MessageUserManger::get().getUserByUserName(loginName);
    if (!userByName)
    {
        LOG_ERROR("{} not exist", loginName);
        return;
    }

    std::shared_ptr<ClientConnection> connection = ClientConnectionManger::get().getConnctionByName(message.attach_data());
    std::string connectionName = connection->name();
    if (!connection)
    {
        LOG_ERROR("client not exist {}", connection->name());
        return;
    }

    PduMessage pdu;
    pdu.setServiceId(IM::BaseDefine::SERVER_ID_LOGIN);
    pdu.setCommandId(IM::BaseDefine::COMMAND_LOGIN_RES_USER_LOGIN);

    IM::Login::LoginResponse response;
    response.set_server_time(mg::TimeStamp::now().getSeconds());
    response.set_result_string(resultString);

    if (result)
    {
        response.set_refuse_type(IM::BaseDefine::REFUSE_REASON_PROXY_VALIDATE_FAILED);
        pdu.setPBMessage(&response);
        connection->setUserConnectionState(1); // close connection state
        connection->send(pdu.dump());
        return;
    }

    userByName->removeUnvalidConnection(connectionName);
    if (userByName->getUnvalidConnectionCount() == 0)
        MessageUserManger::get().removeUserByUserName(loginName);

    IM::BaseDefine::UserInformation information = message.user_info();
    auto userById = MessageUserManger::get().getUserByUserId(information.user_id());

    if (!userById)
    {
        if (userByName->getUnvalidConnectionCount() == 0)
            userById = std::move(userByName);
        else
            userById = std::make_shared<MessageUser>(loginName);
        MessageUserManger::get().addUserByUserId(information.user_id(), userById);
    }

    userById->setValid();
    userById->setUserId(information.user_id());
    MessageUserManger::get().kickOutSameTypeUser(userById->getUserId(), connection->getClientType());
    userById->addValidConnection(connectionName, connection);

    connection->setValid();
    connection->setUserId(userById->getUserId());
    connection->setLoginName(loginName);
    connection->updateUserStatus(IM::BaseDefine::USER_STATUS_ONLINE);

    LOG_INFO("{} {} login success", connection->name(), userById->getUserId());
    response.set_refuse_type(IM::BaseDefine::REFUSE_REASON_NONE);
    IM::BaseDefine::UserInformation *userInfo = response.mutable_user_info();
    userInfo->Swap(&information);
    pdu.setPBMessage(&response);
    connection->send(pdu.dump());
}

void ProxyServerClient::_handleSendMessageResponse(const std::string &data)
{
    IM::Message::MessageData request;
    if (!request.ParseFromString(data))
        return;

    {
        uint32_t user = request.from();
        uint32_t peer = request.to();
        std::string name = request.attach_data();

        auto userConnection = MessageUserManger::get().getConnectionByHandle(user, name).lock();
        if (userConnection)
        {
            IM::Message::MessageDataAck response;
            response.set_from(user);
            response.set_to(peer);
            response.set_session_type(IM::BaseDefine::SESSION_TYPE_SINGLE);

            PduMessage pdu;
            pdu.setServiceId(IM::BaseDefine::SERVER_ID_MESSAGE);
            pdu.setCommandId(IM::BaseDefine::COMMAND_MESSAGE_DATA_ACK);
            pdu.setPBMessage(&response);
            userConnection->send(pdu.dump());
        }
    }

    PduMessage pdu;
    pdu.setServiceId(IM::BaseDefine::SERVER_ID_MESSAGE);
    pdu.setCommandId(IM::BaseDefine::COMMAND_MESSAGE_DATA);
    request.clear_attach_data();
    pdu.setPBMessage(&request);

    std::string sendData = pdu.dump();
    auto user = MessageUserManger::get().getUserByUserId(request.from());
    if (user)
        user->boardcastData(sendData);

    auto peer = MessageUserManger::get().getUserByUserId(request.to());
    if (peer)
        peer->boardcastData(sendData);
}

void ProxyServerClientManger::addConnection(ProxyServerClient *connection)
{
    if (connection == nullptr)
        return;
    this->_clientMemo.push(connection);
}

std::shared_ptr<ProxyServerClient> ProxyServerClientManger::getHandle()
{
    if (this->_clientMemo.empty())
        return std::shared_ptr<ProxyServerClient>();

    std::shared_ptr<ProxyServerClient> connection(this->_clientMemo.front(), [this](ProxyServerClient *ptr)
                                                  { if (ptr) this->_clientMemo.push(ptr); });
    this->_clientMemo.pop();
    return connection;
}

void ProxyServerClientManger::removeConnection(ProxyServerClient *connection)
{
    while (!this->_clientMemo.empty())
    {
        auto top = this->_clientMemo.front();
        this->_clientMemo.pop();
        if (top == connection)
            break;
        this->_clientMemo.push(top);
    }
}