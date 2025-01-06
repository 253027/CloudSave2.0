#include "file-server.h"
#include "../src/log.h"

#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
        return;
    FileServer::get().stop();
    FileServer::destroyInstance();
    ::sleep(1);
    LOG_DEBUG("\r----------------------FileServer exited-----------------------------------");
    ::exit(0);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcasecmp("-daemon", argv[1]))
    {
        if (::daemon(1, 1) == -1)
            return 0;
        std::cout << "FileServer started in daemon mode" << std::endl;
    }
    signal(SIGINT, sighandle);
    signal(SIGTERM, sighandle);

    mg::LogConfig logConfig("debug", "./log", "FileServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------FileServer started-----------------------------------");

    if (!FileServer::get().initial())
        assert(0 && "GatewayServer initial failed");
    FileServer::get().start();

    return 0;
}