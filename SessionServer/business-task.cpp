#include "business-task.h"
#include "../ServerSDK/tcp-packet-parser.h"
#include "../ServerSDK/json.hpp"

void BusinessTask::parse(const mg::TcpConnectionPointer &connection, const std::string &data)
{
    json jsData = json::parse(data);
    mg::TcpPacketParser::getMe().send(connection, data);
    return;
}

void BusinessTask::login(TCPCONNECTION &con, const json &jsData)
{
    ;
}
