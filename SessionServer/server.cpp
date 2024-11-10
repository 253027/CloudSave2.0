#include "session-server.h"
#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT)
        return;
    SessionServer::getMe().quit();
    mg::MysqlConnectionPool::getMe().quit();
    ::sleep(1);
    LOG_DEBUG("\r----------------------SessionServer exited-----------------------------------");
    ::exit(0);
}

int main()
{
    if (::daemon(1, 1) == -1)
        return 0;
    signal(SIGINT, sighandle);

    // 初始化日志库
    mg::LogConfig logConfig("debug", "./log", "SessionServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------SessionServer started-----------------------------------");

    if (!mg::MysqlConnectionPool::getMe().initial("./SessionServer/database.json", "mysql"))
        assert(0 && "mysql initial failed");
    if (!mg::MysqlConnectionPool::getMe().start(180))
        assert(0 && "mysql start failed");

    SessionServer::getMe().initial();
    SessionServer::getMe().start();
    return 0;
}