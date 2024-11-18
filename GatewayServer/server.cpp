#include "gateway-server.h"

#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT)
        return;
    GateWayServer::getMe().quit();
    ::sleep(1);
    LOG_DEBUG("\r----------------------GatewayServer exited-----------------------------------");
    ::exit(0);
}

int main()
{
    if (::daemon(1, 1) == -1)
        return 0;
    signal(SIGINT, sighandle);

    mg::LogConfig logConfig("debug", "./log", "GatewayServer.log");
    INITLOG(logConfig);

    if (!GateWayServer::getMe().initial())
        assert(0 && "GatewayServer initial failed");
    LOG_DEBUG("\r----------------------GatewayServer started-----------------------------------");
    GateWayServer::getMe().start();

    return 0;
}