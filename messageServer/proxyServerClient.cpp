#include "proxyServerClient.h"
#include "../src/base/tcp-client.h"

ProxyServerClient::ProxyServerClient()
{
    ;
}

void ProxyServerClient::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
{
    ;
}

bool ProxyServerClient::connected()
{
    return this->_client->connected();
}

void ProxyServerClientManger::addConnection(ProxyServerClient *connection)
{
    if (connection == nullptr)
        return;
    this->_clientMemo.push(connection);
}

std::shared_ptr<ProxyServerClient> ProxyServerClientManger::getHandle()
{
    if (this->_clientMemo.empty())
        return std::shared_ptr<ProxyServerClient>();

    std::shared_ptr<ProxyServerClient> connection(this->_clientMemo.front(), [this](ProxyServerClient *ptr)
                                                  { if (ptr) this->_clientMemo.push(ptr); });
    this->_clientMemo.pop();
    return connection;
}
