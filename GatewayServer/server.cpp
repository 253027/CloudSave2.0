#include "gateway-server.h"
#include "session-server-client.h"

#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT)
        return;
    GateWayServer::getMe().quit();
    GateWayServer::destroyInstance();
    SessionClient::destroyInstance();
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
    LOG_DEBUG("\r----------------------GatewayServer started-----------------------------------");

    if (!SessionClient::getMe().initial())
        assert(0 && "SessionClient initial failed");

    if (!GateWayServer::getMe().initial())
        assert(0 && "GatewayServer initial failed");
    GateWayServer::getMe().start();

    return 0;
}