#include "loginServerClient.h"
#include "../src/base/tcp-client.h"
#include "../src/base/tcp-connection.h"
#include "../src/base/log.h"
#include "../src/base/tcp-packet-parser.h"
#include "../src/protocal/IM.Server.pb.h"
#include "../src/protocal/imPduBase.h"
#include "../src/protocal/IM.BaseDefine.pb.h"

LoginServerClient::LoginServerClient(int domain, int type, mg::EventLoop *loop,
                                     const mg::InternetAddress &address, const std::string &name)
    : _client(new mg::TcpClient(domain, type, loop, address, name))
{
    _client->setConnectionCallback(std::bind(&LoginServerClient::onConnectionStateChange, this, std::placeholders::_1));
    _client->enableRetry();
}

void LoginServerClient::connect()
{
    this->_client->connect();
}

void LoginServerClient::onConnectionStateChange(const mg::TcpConnectionPointer &link)
{
    if (link->connected())
    {
        // this is a test message
        IM::Server::IMMsgServInfo message;
        char hostname[256] = {0};
        ::gethostname(hostname, 256);
        message.set_ip("127.0.0.1");
        message.set_port(9190);
        message.set_max_conn_cnt(10);
        message.set_cur_conn_cnt(0);
        message.set_host_name(hostname);

        PduMessage pdu;
        pdu.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);
        pdu.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_MSG_SERV_INFO);
        pdu.setPBMessage(&message);
        mg::TcpPacketParser::get().send(link, pdu.dump());

        LOG_INFO("{} success connected to {}", link->name(), link->peerAddress().toIpPort());
    }
    else
    {
        ;
    }
}
