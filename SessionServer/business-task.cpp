#include "business-task.h"
#include "../ServerSDK/tcp-packet-parser.h"
#include "../ServerSDK/json.hpp"

void BusinessTask::parse(const mg::TcpConnectionPointer &connection, const std::string &data)
{
    LOG_DEBUG("data: {}", data);
    json js = json::parse(data);

    METHODTYPE type = js["type"];
    switch (type)
    {
    case METHODTYPE::LOGIN:
        login(connection, js);
        break;
    default:
        break;
    }

    mg::TcpPacketParser::getMe().send(connection, data);
    return;
}

void BusinessTask::login(TCPCONNECTION &con, const json &jsData)
{
    ;
}
