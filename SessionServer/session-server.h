#ifndef __SESSION_SERVER_H__
#define __SESSION_SERVER_H__

#include "business-task.h"
#include "protocal-type.h"

#include "../src/pch.h"
#include "../src/tcp-server.h"
#include "../src/singleton.h"
#include "../src/tcp-packet-parser.h"
#include "../src/log.h"
#include "../src/mysql-connection-pool.h"

class SessionServer : public Singleton<SessionServer>
{
public:
    SessionServer();

    ~SessionServer();

    void initial();

    void start();

    void quit();

private:
    void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

    void onConnectionStateChanged(const mg::TcpConnectionPointer &connection);

    using DataType = Protocal::PackageType;
    std::shared_ptr<mg::EventLoop> _loop;
    std::shared_ptr<mg::TcpServer> _server;
};

#endif //__SESSION_SERVER_H__