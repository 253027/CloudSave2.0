#include "loginServer.h"
#include "../src/log.h"

#include <iostream>

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcasecmp("-daemon", argv[1]))
    {
        if (::daemon(1, 1) == -1)
            return 0;
    }

    mg::LogConfig logConfig("debug", "./log", "LoginServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------LoginServer started-----------------------------------");

    if (!LoginServer::get().initial("./loginServer/loginServer.json"))
        assert(0 && "Initial LoginServer failed");
    if (!LoginServer::get().start())
        assert(0 && "start LoginServer failed");
    return 0;
}