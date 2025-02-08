#ifndef __LOGIN_CONNECTION_H__
#define __LOGIN_CONNECTION_H__

#include "../src/tcp-connection.h"
#include "../src/inet-address.h"

#include <memory>

enum
{
    CONNECTION_HTTP_CLIENT = 1,
    CONNECTION_MESSAGE_SERVER = 2,
    CONNECTION_CLIENT = 3
};

class EventLoop;
class HttpClientConnection : mg::TcpConnection
{
public:
    HttpClientConnection(EventLoop *loop, const std::string &name, int sockfd,
                         const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);
};

#endif //__LOGIN_CONNECTION_H__