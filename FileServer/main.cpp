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
    mg::MysqlConnectionPool::destroyInstance();
    ::sleep(1);
    LOG_DEBUG("\r----------------------FileServer exited-----------------------------------");
    ::exit(0);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcasecmp("-daemon", argv[1]))
    {
        std::cout << "FileServer started in daemon mode" << std::endl;
        if (::daemon(1, 1) == -1)
            return 0;
    }
    signal(SIGINT, sighandle);
    signal(SIGTERM, sighandle);

    mg::LogConfig logConfig("debug", "./log", "FileServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------FileServer started-----------------------------------");

    if (!mg::MysqlConnectionPool::get().initial("./FileServer/database.json", "mysql"))
        assert(0 && "mysql initial failed");
    if (!mg::MysqlConnectionPool::get().start(180))
        assert(0 && "mysql start failed");

    if (!FileServer::get().initial())
        assert(0 && "FileServer initial failed");

    FileServer::get().start();
    return 0;
}