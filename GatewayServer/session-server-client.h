#ifndef __SESSION_SERVER_CLIENT_H__
#define __SESSION_SERVER_CLIENT_H__

#include "../src/singleton.h"
#include "../src/function-callbacks.h"
#include "../src/json_fwd.hpp"

#include <memory>
#include <mutex>
#include <vector>

namespace mg
{
    class TcpClient;
    class EventLoopThread;
    class TcpConnection;
}

class SessionClient : public Singleton<SessionClient>
{
public:
    SessionClient();

    ~SessionClient();

    bool initial();

    bool sendToServer(const std::string &data);

    void quit();

private:
    void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

    void onConnectionStateChanged(const mg::TcpConnectionPointer &connection);

    int _index;
    std::mutex _mutex;
    std::vector<std::weak_ptr<mg::TcpConnection>> _connections;
    std::vector<std::unique_ptr<mg::TcpClient>> _clients;
    std::vector<std::unique_ptr<mg::EventLoopThread>> _threads;
};

#endif //__SESSION_SERVER_CLIENT_H__