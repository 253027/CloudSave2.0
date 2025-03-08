#include "proxyServer.h"
#include "../src/base/log.h"
#include "../src/base/tcp-server.h"

#include <iostream>
#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
        return;
    ProxyServer::get().quit();
    ProxyServer::destroyInstance();
    ProxyServer::destroyInstance();
    LOG_DEBUG("\r----------------------proxyServer exited-----------------------------------");
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

    mg::LogConfig logConfig("debug", "./log", "proxyServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------proxyServer started-----------------------------------");

    if (!ProxyServer::get().initial("./proxyServer/proxyServer.json"))
        assert(0 && "Initial proxyServer failed");
    if (!ProxyServer::get().start())
        assert(0 && "start proxyServer failed");
    return 0;
}