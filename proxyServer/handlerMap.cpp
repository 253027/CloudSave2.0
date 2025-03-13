#include "handlerMap.h"
#include "proxyServer.h"
#include "../src/common/common.h"
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
        LOG_DEBUG("{} heart beat", link->name());
        break;
    }
    case IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_REQ:
        return std::bind(&HandlerMap::login, this, std::move(link), std::move(message));
    }

    return nullptr;
}

void HandlerMap::login(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> &pdu)
{
    ;
}