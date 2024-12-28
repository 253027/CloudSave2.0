#ifndef __FILE_SERVER_H__
#define __FILE_SERVER_H__

#include "../src/singleton.h"
#include "../src/tcp-server.h"
#include "../src/common-server.h"

class FileServer : public CommonServer
{
public:
    FileServer();

    ~FileServer();

    void initial() override;

    void start() override;

    void stop() override;

    void onMessage() override;
};

#endif // __FILE_SERVER_H__