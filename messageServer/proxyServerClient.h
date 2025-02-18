#ifndef __PROXY_SERVER_CLIENT__
#define __PROXY_SERVER_CLIENT__

#include "../src/base/singleton.h"
#include "../src/common/common.h"

#include <memory>
#include <queue>

namespace mg
{
    class TcpClient;
};

class ProxyServerClient : public ConnectionBase
{
public:
    ProxyServerClient();

    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) override;

    bool connected();

private:
    std::unique_ptr<mg::TcpClient> _client;
};

class ProxyServerClientManger : public Singleton<ProxyServerClientManger>
{

public:
    void addConnection(ProxyServerClient *connection);

    std::shared_ptr<ProxyServerClient> getHandle(); // FIXME: not thread safe

private:
    std::queue<ProxyServerClient *> _clientMemo;
};

#endif //__PROXY_SERVER_CLIENT__