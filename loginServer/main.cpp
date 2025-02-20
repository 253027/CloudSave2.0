#include "loginServer.h"
#include "../src/base/log.h"

#include <iostream>
#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
        return;
    LoginServer::get().quit();
    LoginServer::destroyInstance();
    LOG_DEBUG("\r----------------------LoginServer exited-----------------------------------");
    SHUTDOWNLOG();
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

    mg::LogConfig logConfig("debug", "./log", "loginServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------LoginServer started-----------------------------------");

    if (!LoginServer::get().initial("./loginServer/loginServer.json"))
        assert(0 && "Initial LoginServer failed");
    if (!LoginServer::get().start())
        assert(0 && "start LoginServer failed");
    return 0;
}