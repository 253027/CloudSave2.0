#include "session-server.h"
#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT)
        return;
    SessionServer::getMe().quit();
    sleep(1);
    LOG_DEBUG("\r----------------------SessionServer exited-----------------------------------");
    exit(0);
}

int main()
{
    // 初始化日志库
    mg::LogConfig logConfig("debug", "../log", "SessionServer.log");
    INITLOG(logConfig);
    signal(SIGINT, sighandle);
    LOG_DEBUG("\r----------------------SessionServer started-----------------------------------");
    SessionServer::getMe().initial();
    SessionServer::getMe().start();
    return 0;
}