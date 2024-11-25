#include "json-data-parser.h"
#include "../ServerSDK/json.hpp"
#include "../ServerSDK/log.h"
#include "session-server-client.h"

#include <string.h>

using json = nlohmann::json;

std::string packet(int type, const std::string &data)
{
    std::string temp(4, '0');
    ::memcpy(temp.data(), &type, sizeof(type));
    return temp + data;
}

JsonDataParser::JsonDataParser()
{
    ;
}

void JsonDataParser::parse(const std::string &name, std::string &data)
{
    json js;
    try
    {
        js = json::parse(data);
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("{}", e.what());
        return;
    }

    if (!js.contains("type") || !js["type"].is_string())
    {
        LOG_ERROR("{} invalid argument type", name);
        return;
    }

    const std::string &type = js["type"];
    js.erase("type"), js["connection-name"] = name;
    bool ret = true;

    if (type == "login")
        ret = SessionClient::getMe().sendToServer(packet(3, js.dump()));
    else if (type == "regist")
        ;

    if (!ret)
        LOG_ERROR("{} send data failed, type: {}", name, type);
}