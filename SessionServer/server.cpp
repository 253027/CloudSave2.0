#include "session-server.h"
#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    SessionServer::getMe().quit();
    mg::MysqlConnectionPool::getMe().quit();

    SessionServer::destroyInstance();
    mg::MysqlConnectionPool::destroyInstance();
    LOG_DEBUG("\r----------------------SessionServer exited-----------------------------------");
    SHUTDOWNLOG();
    // sleep(1);
    ::exit(0);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcasecmp("-daemon", argv[1]))
    {
        if (::daemon(1, 1) == -1)
            return 0;
        std::cout << "SessionServer started in daemon mode" << std::endl;
    }
    signal(SIGINT, sighandle);
    signal(SIGTERM, sighandle);

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