#ifndef __GATEWAY_SERVER_H__
#define __GATEWAY_SERVER_H__

#include "../src/singleton.h"
#include "../src/function-callbacks.h"
#include "../src/time-stamp.h"
#include "../src/json_fwd.hpp"

#include "../src/log.h"

#include <memory>
#include <unordered_map>
#include <mutex>

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

    void onInternalServerResponse(const std::string &name, nlohmann::json &js);

private:
    void regist();

    void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

    void connectionStateChange(const mg::TcpConnectionPointer &a);

    void invalidResponse(const mg::TcpConnectionPointer &a);

    std::unique_ptr<mg::TcpServer> _server;
    std::unique_ptr<mg::EventLoop> _loop;
    std::mutex _mutex;
    std::unordered_map<std::string, std::weak_ptr<mg::TcpConnection>> _connection;
};

#endif //__GATEWAY_SERVER_H__