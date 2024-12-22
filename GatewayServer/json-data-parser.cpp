#include "json-data-parser.h"
#include "session-server-client.h"
#include "../src/json-extract.h"
#include "../src/log.h"
#include "../src/tcp-connection.h"
#include "../protocal/protocal-session.h"

#include <string.h>
using namespace Protocal;

using json = nlohmann::json;

JsonDataParser::JsonDataParser()
{
    ;
}

bool JsonDataParser::parse(const mg::TcpConnectionPointer &a, std::string &data)
{
    json js;
    try
    {
        js = json::parse(data);
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("{}", e.what());
        return false;
    }

    std::string type;
    if (!mg::JsonExtract::extract(js, "type", type, mg::JsonExtract::STRING))
    {
        LOG_ERROR("{} invalid argument type", a->name());
        return false;
    }

    js["connection-name"] = a->name();
    js["con-state"] = a->getUserConnectionState();
    bool valid = true;

    switch (this->_method[type])
    {
    case Method::LOGIN:
        valid = SessionClient::get().sendToServer(SessionCommand(SessionType::LOGIN).serialize(js.dump()));
        break;
    case Method::REGIST:
        break;
    case Method::UPLOAD:
        valid = SessionClient::get().sendToServer(SessionCommand(SessionType::UPLOAD).serialize(js.dump()));
        break;
    default:
        valid = false;
        break;
    }

    return valid;
}