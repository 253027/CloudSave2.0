#include "clientConnection.h"
#include "messageServer.h"
#include "messageUser.h"
#include "proxyServerClient.h"
#include "../src/base/log.h"
#include "../src/common/common-macro.h"

ClientConnection::ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                   const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : ConnectionBase(), TcpConnection(loop, name, sockfd, localAddress, peerAddress),
      _loginName(), _userId(0), _clientType(0)
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
        LOG_INFO("new client connection:{}", link->name());
    }
    else
    {
        ClientConnectionManger::get().removeConnection(link->name());
    }
}

void ClientConnection::writeCompleteCallback(const mg::TcpConnectionPointer &link)
{
    LOG_DEBUG("{} write complete", link->name());
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

        data = message->getPBmessage().retrieveAllAsString();
        switch (message->getCommandId())
        {
        case IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT:
        {
            this->setNextReceiveTime(mg::TimeStamp(time.getMircoSecond() + SERVER_TIMEOUT));
            LOG_DEBUG("{} heart beat message", link->name());
            break;
        }
        case IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_REQ:
        {
            this->handleLoginRequest(data);
            break;
        }
        }
    }
}

void ClientConnection::handleLoginRequest(const std::string &data)
{
    if (!this->_loginName.empty())
    {
        LOG_DEBUG("{} duplicate login request in same connection", this->name());
        return;
    }

    PARSE_PROTOBUF_MESSAGE(IM::Login::LoginRequest, request, data);

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
        response.set_result_string(resultString);
        message.setServiceId(IM::BaseDefine::SERVER_ID_LOGIN);
        message.setCommandId(IM::BaseDefine::COMMAND_LOGIN_RES_USER_LOGIN);
        message.setPBMessage(&response);
        mg::TcpPacketParser::get().send(shared_from_this(), message.dump());
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
    mg::TcpPacketParser::get().send(connection->connection(), messagePdu.dump()); // send to proxy server valid this user
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

    auto connection = it->second.lock();
    if (!connection)
        return std::shared_ptr<ClientConnection>();

    return std::dynamic_pointer_cast<ClientConnection>(connection);
}