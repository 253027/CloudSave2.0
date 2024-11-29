#include "json-data-parser.h"
#include "../ServerSDK/json.hpp"
#include "../ServerSDK/log.h"
#include "session-server-client.h"

#include <string.h>

using json = nlohmann::json;

JsonDataParser::JsonDataParser()
{
    ;
}

bool JsonDataParser::parse(const std::string &name, std::string &data)
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

    if (!js.contains("type") || !js["type"].is_string())
    {
        LOG_ERROR("{} invalid argument type", name);
        return false;
    }

    const std::string &type = js["type"];
    js.erase("type");
    js["connection-name"] = name;
    bool valid = true;

    switch (this->_method[type])
    {
    case Method::LOGIN:
        valid = SessionClient::getMe().sendToServer(packet(3, js.dump()));
        break;
    case Method::REGIST:
        break;
    default:
        valid = false;
        break;
    }

#ifdef _DEBUG
    if (!valid)
        LOG_ERROR("{} send data failed, type: {}", name, type);
#endif
    return valid;
}

std::string JsonDataParser::packet(int type, const std::string &data)
{
    std::string temp(4, '0');
    ::memcpy(temp.data(), &type, sizeof(type));
    return temp + data;
}
