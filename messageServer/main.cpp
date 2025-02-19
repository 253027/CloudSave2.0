#include "messageServer.h"
#include "loginServerClient.h"
#include "../src/base/log.h"
#include "../src/base/tcp-server.h"

#include <iostream>
#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
        return;
    MessageServer::get().quit();
    MessageServer::destroyInstance();
    ::sleep(1);
    LOG_DEBUG("\r----------------------MessageServer exited-----------------------------------");
    ::exit(0);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcasecmp("-daemon", argv[1]))
    {
        if (::daemon(1, 1) == -1)
            return 0;
    }
    signal(SIGINT, sighandle);
    signal(SIGTERM, sighandle);

    mg::LogConfig logConfig("debug", "./log", "messageServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------MessageServer started-----------------------------------");

    if (!MessageServer::get().initial("./messageServer/messageServer.json"))
        assert(0 && "Initial messageServer failed");
    if (!MessageServer::get().start())
        assert(0 && "start messageServer failed");
    return 0;
}