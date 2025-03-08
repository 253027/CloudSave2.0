#include "proxyServer.h"
#include "../src/base/tcp-server.h"
#include "../src/base/event-loop.h"
#include "../src/base/log.h"
#include "../src/base/json.hpp"

#include <fstream>

using json = nlohmann::json;
bool ProxyServer::initial(const std::string &configPath)
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

    this->_loop.reset(new mg::EventLoop("MainEventLoop"));
    this->_server.reset(new mg::TcpServer(this->_loop.get(),
                                          mg::InternetAddress(static_cast<std::string>(config["ip"]), static_cast<uint16_t>(config["port"])),
                                          "ProxyServer"));
    this->_server->setThreadNums(static_cast<int>(config["threadNums"]));
    return true;
}

bool ProxyServer::start()
{
    return true;
}

void ProxyServer::quit()
{
    ;
}
