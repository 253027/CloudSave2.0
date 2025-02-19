#include "proxyServerClient.h"
#include "../src/base/tcp-client.h"

ProxyServerClient::ProxyServerClient()
{
    ;
}

bool ProxyServerClient::connected()
{
    return this->_client->connected();
}

mg::TcpConnectionPointer ProxyServerClient::connection()
{
    return this->_client->connection();
}

void ProxyServerClient::messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time)
{
    while (1)
    {
        std::string data;
        if (!mg::TcpPacketParser::get().reveive(link, data))
            break;

        std::unique_ptr<PduMessage> message(new PduMessage());
        if (!message->parse(data))
        {
            LOG_ERROR("{} wrong message", link->name());
            continue;
        }

        switch (message->getCommandId())
        {
        case IM::BaseDefine::COMMAND_ID_OTHER_HEARTBEAT:
        {
            this->setNextReceiveTime(mg::TimeStamp(time.getMircoSecond() + SERVER_HEARTBEAT_INTERVAL));
            LOG_DEBUG("{} heart beat message", link->name());
            break;
        }
        case IM::BaseDefine::COMMAND_ID_OTHER_VALIDATE_RSP:
        {
            this->_handleVerifyDataResponse(message->getPBmessage().retrieveAllAsString());
            break;
        }
        }
    }
}

void ProxyServerClient::_handleVerifyDataResponse(const std::string &data)
{
    IM::Server::VerifyDataResponse message;
    if (!message.ParseFromString(data))
    {
        LOG_ERROR("{} parse protobuf message error", this->_client->connection()->name());
        return;
    }
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
