#include "handlerMap.h"
#include "proxyServer.h"
#include "login.h"
#include "../src/common/common.h"
#include "../src/base/log.h"

mg::Handler HandlerMap::getCallBack(const mg::TcpConnectionPointer &link, std::string &data)
{
    std::shared_ptr<PduMessage> message(new PduMessage());
    if (!message->parse(data))
        return nullptr;

    data = message->getPBmessage().retrieveAllAsString();
    switch (message->getCommandId())
    {
    case IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT:
    {
        heartBeatManger[link->name()] = mg::TimeStamp(mg::TimeStamp::now().getMircoSecond() + SERVER_TIMEOUT);
        LOG_DEBUG("{} heart beat", link->name());
        break;
    }
    case IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_REQ:
        return std::bind(&HandlerMap::login, this, std::move(link), std::move(data));
    }

    return nullptr;
}

void HandlerMap::login(const mg::TcpConnectionPointer &link, const std::string &data)
{
    IM::Server::VerifyDataRequest request;
    PduMessage message;
    message.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);
    message.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_RSP);
    IM::Server::VerifyDataResponse response;

    if (request.ParseFromString(std::move(data)))
    {
        std::string userName = request.user_name();
        std::string password = request.password();

        IM::BaseDefine::UserInformation *info = response.mutable_user_info();
        *response.mutable_attach_data() = std::move(request.attach_data());
        *response.mutable_user_name() = userName;

        if (!Login::get().doLogin(userName, password, *info))
        {
            response.clear_user_info();
            response.set_result_code(1);
            response.set_result_string("user not exit or password error!");
        }
        else
        {
            response.set_result_code(0);
            response.set_result_string("login success!");
        }
    }
    else
    {
        response.set_result_code(2);
        response.set_result_string("Internal Server Error!");
    }

    message.setPBMessage(&response);
    mg::TcpPacketParser::get().send(link, message.dump());
}