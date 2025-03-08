#ifndef __PROXY_SERVER_H__
#define __PROXY_SERVER_H__

#include "../src/base/singleton.h"

#include <string>
#include <memory>

namespace mg
{
    class TcpServer;
    class EventLoop;
}

class ProxyServer : public Singleton<ProxyServer>
{
public:
    bool initial(const std::string &configPath);

    bool start();

    void quit();

private:
    std::unique_ptr<mg::TcpServer> _server;
    std::shared_ptr<mg::EventLoop> _loop;
};

#endif //__PROXY_SERVER_H__
