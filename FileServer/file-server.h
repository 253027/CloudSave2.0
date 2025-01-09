#ifndef __FILE_SERVER_H__
#define __FILE_SERVER_H__

#include "../src/singleton.h"
#include "../src/tcp-server.h"

class FileServer : public Singleton<FileServer>
{
public:
    FileServer();

    ~FileServer();

    bool initial();

    void start();

    void stop();

    void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

private:
    std::shared_ptr<mg::TcpServer> _server;
    std::shared_ptr<mg::EventLoop> _loop;
};

#endif // __FILE_SERVER_H__