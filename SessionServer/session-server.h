#ifndef __SESSION_SERVER_H__
#define __SESSION_SERVER_H__

#include "../ServerSDK/pch.h"
#include "../ServerSDK/tcp-server.h"
#include "../ServerSDK/singleton.h"
#include "../ServerSDK/tcp-packet-parser.h"
#include "../ServerSDK/log.h"

class SessionServer : public Singleton<SessionServer>
{
public:
    void onConnectionStateChanged(const mg::TcpConnectionPointer &connection);

    void initial();

    void start();

    void quit();

    void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

private:
    std::shared_ptr<mg::EventLoop> _loop;
    std::shared_ptr<mg::TcpServer> _server;
};

#endif //__SESSION_SERVER_H__