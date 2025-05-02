#include "handlerMap.h"
#include "proxyServer.h"
#include "login.h"
#include "session.h"
#include "user.h"
#include "messageCache.h"
#include "../src/base/log.h"

mg::Handler HandlerMap::getCallBack(const mg::TcpConnectionPointer &link, std::string &data)
{
    std::shared_ptr<PduMessage> message(new PduMessage());
    if (!message->parse(data))
        return nullptr;

    switch (message->getCommandId())
    {
    case IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT:
    {
        heartBeatManger[link->name()] = mg::TimeStamp(mg::TimeStamp::now().getMircoSecond() + SERVER_TIMEOUT);
        LOG_TRACE("{} heart beat", link->name());
        break;
    }
    case IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_REQ:
        return std::bind(&HandlerMap::login, this, std::move(link), std::move(message));
    case IM::BaseDefine::COMMAND_MESSAGE_DATA:
        return std::bind(&HandlerMap::sendMessage, this, std::move(link), std::move(message));
    case IM::BaseDefine::COMMAND_ID_FRIEND_LIST_FRIEND_REQ:
        return std::bind(&HandlerMap::getChangedFriendList, this, std::move(link), std::move(message));
    }

    return nullptr;
}

void HandlerMap::login(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> data)
{
    PduMessage message;
    message.setSequenceNumber(data->getSequenceNumber());
    message.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);
    message.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_RSP);
    IM::Server::VerifyDataResponse response;

    IM::Server::VerifyDataRequest request;
    if (request.ParseFromString(data->getPBmessage().retrieveAllAsString()))
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

void HandlerMap::getChangedFriendList(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> data)
{
    IM::Buddy::IMGetFriendListRequest request;
    if (!request.ParseFromString(data->getPBmessage().retrieveAllAsString()))
        return;

    IM::Buddy::IMGetFriendListResponse response;
    response.set_user_id(request.user_id());
    response.set_latest_update_time(mg::TimeStamp::now().getSeconds());
    response.set_attach_data(request.attach_data());

    std::vector<uint32_t> list;
    User::get().getFriendsList(request.user_id(), list, request.last_update_time());

    for (auto &id : list)
    {
        IM::BaseDefine::UserInformation info;
        if (!User::get().getFriendsInfo(id, info))
            continue;
        response.add_user_list()->CopyFrom(info);
    }

    PduMessage pdu;
    pdu.setCommandId(IM::BaseDefine::COMMAND_ID_FRIEND_LIST_FRIEND_RES);
    pdu.setServiceId(IM::BaseDefine::SERVER_ID_BUDDY_LIST);
    pdu.setSequenceNumber(data->getSequenceNumber());
    pdu.setPBMessage(&response);
    mg::TcpPacketParser::get().send(link, pdu.dump());
}

void HandlerMap::sendMessage(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> data)
{
    IM::Message::MessageData request;
    if (!request.ParseFromString(data->getPBmessage().retrieveAllAsString()))
        return;

    switch (request.message_type())
    {
    case IM::BaseDefine::MESSAGE_TYPE_SINGLE_TEXT:
    {
        uint32_t messageId = this->sendSingleMessage(request);
        if (!messageId)
            return;
        request.set_messsage_id(messageId);
        break;
    }
    }

    PduMessage response;
    response.setServiceId(IM::BaseDefine::SERVER_ID_MESSAGE);
    response.setCommandId(IM::BaseDefine::COMMAND_MESSAGE_DATA);
    response.setSequenceNumber(data->getSequenceNumber());
    response.setPBMessage(&request);
    mg::TcpPacketParser::get().send(link, response.dump());
}

uint32_t HandlerMap::sendSingleMessage(IM::Message::MessageData &request)
{
    uint32_t from = request.from();
    uint32_t to = request.to();
    if (from == to)
        return 0;

    uint32_t userSession = Session::get().getSession(from, to, IM::BaseDefine::MESSAGE_TYPE_SINGLE_TEXT);
    if (!userSession)
        userSession = Session::get().addSession(from, to, IM::BaseDefine::MESSAGE_TYPE_SINGLE_TEXT);

    uint32_t peerSession = Session::get().getSession(to, from, IM::BaseDefine::MESSAGE_TYPE_SINGLE_TEXT);
    if (!peerSession)
        peerSession = Session::get().addSession(to, from, IM::BaseDefine::MESSAGE_TYPE_SINGLE_TEXT);

    if (!peerSession || !userSession)
        return 0;

    uint32_t relation = Session::get().getRelation(from, to, true);
    if (!relation)
        relation = Session::get().addRelation(from, to);
    Session::get().saveMessage(relation, request);

    return MessageCache::get().getMessageId(relation);
}