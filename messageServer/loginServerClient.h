#ifndef __LOGIN_SERVER_CLIENT_H__
#define __LOGIN_SERVER_CLIENT_H__

#include "../src/base/function-callbacks.h"
#include "../src/base/event-loop.h"
#include "../src/base/inet-address.h"

#include <memory>
namespace mg
{
    class TcpClient;
}

class LoginServerClient
{
public:
    LoginServerClient(int domain, int type, mg::EventLoop *loop,
                      const mg::InternetAddress &address, const std::string &name);

    void connect();

    void onConnectionStateChange(const mg::TcpConnectionPointer &link);

private:
    std::unique_ptr<mg::TcpClient> _client;
};

#endif //__LOGIN_SERVER_CLIENT_H__