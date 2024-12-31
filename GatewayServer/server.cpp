#include "gateway-server.h"
#include "session-server-client.h"

#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
        return;
    GateWayServer::get().quit();
    GateWayServer::destroyInstance();
    SessionClient::destroyInstance();
    ::sleep(1);
    LOG_DEBUG("\r----------------------GatewayServer exited-----------------------------------");
    ::exit(0);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcasecmp("-daemon", argv[1]))
    {
        if (::daemon(1, 1) == -1)
            return 0;
        std::cout << "GatewayServer started in daemon mode" << std::endl;
    }
    signal(SIGINT, sighandle);
    signal(SIGTERM, sighandle);

    mg::LogConfig logConfig("debug", "./log", "GatewayServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------GatewayServer started-----------------------------------");

    if (!SessionClient::get().initial())
        assert(0 && "SessionClient initial failed");

    if (!GateWayServer::get().initial())
        assert(0 && "GatewayServer initial failed");
    GateWayServer::get().start();

    return 0;
}