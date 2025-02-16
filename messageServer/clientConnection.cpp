#include "clientConnection.h"
#include "messageServer.h"
#include "messageUser.h"
#include "../src/base/log.h"
#include "../src/protocal/IM.BaseDefine.pb.h"
#include "../src/protocal/IM.Login.pb.h"

ClientConnection::ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                                   const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress)
    : ConnectionBase(), TcpConnection(loop, name, sockfd, localAddress, peerAddress),
      _loginName(), _userId(0), _clientType(0)
{
    ;
}

void ClientConnection::handleLoginRequest(PduMessage *message)
{
    if (!this->_loginName.empty())
    {
        LOG_DEBUG("{} duplicate login request in same connection", this->name());
        return;
    }

    uint32_t result = 0;
    std::string resultString = "Server abnormality";
    // TODO: get databases connection handle

    if (!MessageServer::get().loginServerAvaiable())
        result = IM::BaseDefine::REFUSE_REASON_NO_LOGIN_SERVER;
    if (result)
        return;

    IM::Login::IMLoginRequest request;
    if (!request.ParseFromString(message->getPBmessage().retrieveAllAsString()))
    {
        LOG_ERROR("{} parse message error", this->name());
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
    user->addUnValidConnection(request.user_name(), std::dynamic_pointer_cast<ClientConnection>(shared_from_this()));
}