#ifndef __PROXY_SERVER_CLIENT__
#define __PROXY_SERVER_CLIENT__

#include "../src/base/function-callbacks.h"
#include "../src/base/singleton.h"
#include "../src/common/common.h"

#include <memory>
#include <queue>

namespace mg
{
    class TcpClient;
    class EventLoop;
};

class PduMessage;

class ProxyServerClient : public ConnectionBase
{
public:
    ProxyServerClient(int domain, int type, mg::EventLoop *loop,
                      const mg::InternetAddress &address, const std::string &name);

    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) override;

    void connectionChangeCallback(const mg::TcpConnectionPointer &link, mg::EventLoop *loop);

    void connect();

    bool connected();

    mg::TcpConnectionPointer connection();

    void send(const std::string &data);

private:
    void _handleVerifyDataResponse(const std::string &data);

private:
    std::unique_ptr<mg::TcpClient> _client;
};

class ProxyServerClientManger : public Singleton<ProxyServerClientManger>
{

public:
    void addConnection(ProxyServerClient *connection);

    std::shared_ptr<ProxyServerClient> getHandle(); // FIXME: not thread safe

    void removeConnection(ProxyServerClient *connection);

private:
    std::queue<ProxyServerClient *> _clientMemo;
};

#endif //__PROXY_SERVER_CLIENT__