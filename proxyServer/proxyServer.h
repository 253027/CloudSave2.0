#ifndef __PROXY_SERVER_H__
#define __PROXY_SERVER_H__

#include "../src/base/singleton.h"
#include "../src/base/time-stamp.h"
#include "../src/base/timer-id.h"
#include "../src/base/http-method-call.h"
#include "../src/common/common.h"

#include <string>
#include <memory>
#include <unordered_map>

namespace mg
{
    class TcpServer;
    class EventLoop;
    class ThreadPool;
    class Buffer;
}

class ProxyServer : public Singleton<ProxyServer>
{
public:
    bool initial(const std::string &configPath);

    bool start();

    void quit();

private:
    void onMessage(const mg::TcpConnectionPointer &link, mg::Buffer *b, mg::TimeStamp c);

    void onConnectionState(const mg::TcpConnectionPointer &link);

    void heartBeatMessage(const mg::TcpConnectionPointer &link);

private:
    std::unique_ptr<mg::TcpServer> _server;
    std::shared_ptr<mg::EventLoop> _loop;
    std::unique_ptr<mg::ThreadPool> _threadPool;
};

extern thread_local std::unordered_map<std::string, mg::TimeStamp> heartBeatManger;
#endif //__PROXY_SERVER_H__
