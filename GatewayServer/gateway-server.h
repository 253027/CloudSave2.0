#ifndef __GATEWAY_SERVER_H__
#define __GATEWAY_SERVER_H__

#include "../ServerSDK/singleton.h"
#include "../ServerSDK/function-callbacks.h"
#include "../ServerSDK/time-stamp.h"

#include "../ServerSDK/log.h"

#include <memory>
#include <unordered_map>

namespace mg
{
    class TcpServer;
    class EventLoop;
}

class GateWayServer : public Singleton<GateWayServer>
{
public:
    GateWayServer();

    ~GateWayServer();

    bool initial();

    void start();

    void quit();

    void onInternalServerResponse(const std::string &name, std::string &data);

private:
    void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

    void connectionStateChange(const mg::TcpConnectionPointer &a);

    std::unique_ptr<mg::TcpServer> _server;
    std::unique_ptr<mg::EventLoop> _loop;
    std::unordered_map<std::string, std::weak_ptr<mg::TcpConnection>> _connection;
};

#endif //__GATEWAY_SERVER_H__