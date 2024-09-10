#include "../ServerSDK/pch.h"

int main()
{
    // 初始化日志库
    mg::LogConfig logConfig("debug", "./log", "server.log");
    INITLOG(logConfig);

    return 0;
}