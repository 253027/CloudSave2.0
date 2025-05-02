#include "proxyServer.h"
#include "../src/base/log.h"
#include "../src/base/tcp-server.h"
#include "../src/base/threadpool.h"
#include "../src/base/mysql-connection-pool.h"
#include "../src/base/json.hpp"

#include "../src/common/common-macro.h"

#include <fstream>
#include <iostream>
#include <unistd.h>
#include <csignal>

void sighandle(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
        return;
    ProxyServer::get().quit();
    ProxyServer::destroyInstance();
    LOG_INFO("\r----------------------proxyServer exited-----------------------------------");
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
    LOG_INFO("\r----------------------proxyServer started-----------------------------------");

    PARSE_JSON_FILE(js, "./proxyServer/proxyServer.json");
    if (!mg::MysqlConnectionPool::get().initial(js["mysql"], "mysql"))
        assert(0 && "mysql initial failed");
    if (!mg::MysqlConnectionPool::get().start(180))
        assert(0 && "mysql start failed");

    if (!ProxyServer::get().initial("./proxyServer/proxyServer.json"))
        assert(0 && "Initial proxyServer failed");
    if (!ProxyServer::get().start())
        assert(0 && "start proxyServer failed");
    return 0;
}