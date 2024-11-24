#ifndef __SESSION_SERVER_CLIENT_H__
#define __SESSION_SERVER_CLIENT_H__

#include "../ServerSDK/singleton.h"
#include "../ServerSDK/function-callbacks.h"

#include <memory>
#include <mutex>
#include <vector>

namespace mg
{
    class TcpClient;
    class EventLoopThread;
}

class SessionClient : public Singleton<SessionClient>
{
public:
    SessionClient();

    ~SessionClient();

    bool initial();

private:
    void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

    void onConnectionStateChanged(const mg::TcpConnectionPointer &connection);

    std::mutex _mutex;
    std::vector<std::unique_ptr<mg::TcpClient>> _clients;
    std::vector<std::unique_ptr<mg::EventLoopThread>> _threads;
};

#endif //__SESSION_SERVER_CLIENT_H__