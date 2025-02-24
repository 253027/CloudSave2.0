#include "proxyServerClient.h"
#include "clientConnection.h"
#include "messageUser.h"
#include "../src/base/tcp-client.h"
#include "../src/base/log.h"
#include "../src/protocal/IM.Login.pb.h"

ProxyServerClient::ProxyServerClient()
{
    ;
}

bool ProxyServerClient::connected()
{
    return this->_client->connected();
}

mg::TcpConnectionPointer ProxyServerClient::connection()
{
    return this->_client->connection();
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

        switch (message->getCommandId())
        {
        case IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT:
        {
            this->setNextReceiveTime(mg::TimeStamp(time.getMircoSecond() + SERVER_TIMEOUT));
            LOG_DEBUG("{} heart beat message", link->name());
            break;
        }
        case IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_RSP:
        {
            this->_handleVerifyDataResponse(message->getPBmessage().retrieveAllAsString());
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

    if (!result)
    {
        IM::Login::LoginResponse response;
        response.set_server_time(mg::TimeStamp::now().getSeconds());
        response.set_result_string(resultString);

        PduMessage pdu;
        pdu.setServiceId(IM::BaseDefine::SERVER_ID_LOGIN);
        pdu.setCommandId(IM::BaseDefine::COMMAND_LOGIN_RES_USER_LOGIN);
        pdu.setPBMessage(&response);
        connection->setUserConnectionState(1); // close connection state
        mg::TcpPacketParser::get().send(connection, pdu.dump());
        return;
    }

    result = IM::BaseDefine::REFUSE_REASON_PROXY_VALIDATE_FAILED;
    IM::BaseDefine::UserInformation information = message.user_info();
    auto userById = MessageUserManger::get().getUserByUserId(information.user_id());
    if (userById)
    {
        userByName->removeUnvalidConnection(connectionName);
    }
    else
        userById = userByName;
    userById->setValid();
    userById->addValidConnection(connectionName, connection);
    userById->removeUnvalidConnection(connectionName);

    // TODO: keep one facility online
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
