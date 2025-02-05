#include "loginServer.h"
#include "../src/log.h"
#include "../src/json.hpp"

#include <fstream>

using json = nlohmann::json;

bool LoginServer::initial(const std::string &configPath)
{
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        LOG_ERROR("read configuration File failed");
        return false;
    }

    json config;
    try
    {
        file >> config;
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("configPath parse error: {}", e.what());
        return false;
    }

    return true;
}

bool LoginServer::start()
{
    return true;
}

void LoginServer::quit()
{
    ;
}
