#ifndef __HANDLER_MAP_H__
#define __HANDLER_MAP_H__

#include "../src/base/singleton.h"
#include "../src/base/function-callbacks.h"
#include "../src/common/common.h"

#include <memory>
#include <unordered_map>

class PduMessage;
class HandlerMap : public Singleton<HandlerMap>
{
public:
    mg::Handler getCallBack(const mg::TcpConnectionPointer &link, std::string &data);

private:
    void login(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> data);

    void getChangedFriendList(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> data);

    void sendMessage(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> data);
    uint32_t sendSingleMessage(IM::Message::MessageData &message);

    void getUnReadMessageCount(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> data);

    void handleMessageDataAck(const mg::TcpConnectionPointer &link, std::shared_ptr<PduMessage> data);
};

#endif // __HANDLER_MAP_H__