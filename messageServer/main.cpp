#include "messageServer.h"
#include "../src/base/log.h"

#include <iostream>

int main(int argc, char *argv[])
{
    if (argc > 1 && !strcasecmp("-daemon", argv[1]))
    {
        if (::daemon(1, 1) == -1)
            return 0;
    }

    mg::LogConfig logConfig("debug", "./log", "MessageServer.log");
    INITLOG(logConfig);
    LOG_DEBUG("\r----------------------MessageServer started-----------------------------------");

    if (!MessageServer::get().initial("./messageServer/messageServer.json"))
        assert(0 && "Initial messageServer failed");
    if (!MessageServer::get().start())
        assert(0 && "start messageServer failed");
    return 0;
}