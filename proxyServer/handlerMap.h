#ifndef __HANDLER_MAP_H__
#define __HANDLER_MAP_H__

#include "../src/base/singleton.h"
#include "../src/base/function-callbacks.h"

#include <memory>
#include <unordered_map>

class PduMessage;
class HandlerMap : public Singleton<HandlerMap>
{
public:
    mg::Handler getCallBack(const mg::TcpConnectionPointer &link, std::string &data);

private:
    void login(const mg::TcpConnectionPointer &link, const std::string &data);

    void sendMessage(const mg::TcpConnectionPointer &link, const std::string &data);
};

#endif // __HANDLER_MAP_H__