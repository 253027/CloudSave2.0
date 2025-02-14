#ifndef __LOGIN_SERVER_CLIENT_H__
#define __LOGIN_SERVER_CLIENT_H__

#include "../src/base/function-callbacks.h"
#include "../src/base/event-loop.h"
#include "../src/base/inet-address.h"
#include "../src/common/common.h"
#include "../src/base/singleton.h"

#include <memory>

namespace mg
{
    class TcpClient;
}

class LoginServerClient : public ConnectionBase
{
public:
    LoginServerClient(int domain, int type, mg::EventLoop *loop,
                      const mg::InternetAddress &address, const std::string &name);

    void connect();

    void onConnectionStateChange(const mg::TcpConnectionPointer &link, mg::EventLoop *loop);

    void onMessage(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time);

    void heartBeatMessage(const mg::TcpConnectionPointer &link);

private:
    void send(const mg::TcpConnectionPointer &link, const std::string &data);

private:
    std::unique_ptr<mg::TcpClient> _client;
};

#endif //__LOGIN_SERVER_CLIENT_H__