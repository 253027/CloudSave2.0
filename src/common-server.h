#ifndef __COMMON_SERVER_H__
#define __COMMON_SERVER_H__

#include "singleton.h"
#include "tcp-server.h"

class CommonServer : public Singleton<CommonServer>
{
public:
    virtual ~CommonServer() {};

    virtual bool initial();

    virtual void start();

    virtual void stop();

protected:
    virtual void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

    std::shared_ptr<mg::TcpServer> _server;
    std::shared_ptr<mg::EventLoop> _loop;
};

#endif //__COMMON_SERVER_H__