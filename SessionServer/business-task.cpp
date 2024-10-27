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
    case METHODTYPE::REGIST:
        regist(connection, js);
        break;
    default:
        break;
    }

    mg::TcpPacketParser::getMe().send(connection, data);
    return;
}

void BusinessTask::login(TCPCONNECTION &con, const json &jsData)
{
    int state = con->getUserConnectionState();
    if (state != CONNECTIONSTATE::UNVERIFY)
        return;

    std::string name = jsData["name"];
    std::string password = jsData["password"];
    if (name.empty() || password.empty())
        return;

    // TODO: 查数据库操作

    con->setUserConnectionState(CONNECTIONSTATE::VERIFY);
}

void BusinessTask::regist(TCPCONNECTION &con, const json &jsData)
{
    ;
}
