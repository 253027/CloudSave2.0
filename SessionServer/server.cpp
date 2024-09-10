#include "../ServerSDK/mg_pch.h"
#include "../ServerSDK/mg_connection.h"

int main()
{
    // 初始化日志库
    mg::LogConfig logConfig("debug", "./log", "server.log");
    INITLOG(logConfig);

    mg::Connection connect("127.0.0.1", 9190, mg::IPV4_DOMAIN, mg::TCP_SOCKET);

    return 0;
}