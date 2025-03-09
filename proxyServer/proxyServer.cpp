#include "proxyServer.h"
#include "handlerMap.h"
#include "../src/base/tcp-server.h"
#include "../src/base/event-loop.h"
#include "../src/base/log.h"
#include "../src/base/json.hpp"
#include "../src/base/threadpool.h"
#include "../src/base/tcp-packet-parser.h"
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
                                          mg::InternetAddress(static_cast<std::string>(config["listenIp"]),
                                                              static_cast<uint16_t>(config["listenPort"])),
                                          "ProxyServer"));
    this->_server->setThreadNums(static_cast<int>(config["threadNums"]));
    this->_threadPool.reset(new mg::ThreadPool("ProxyServerThreadPool"));
    return true;
}

bool ProxyServer::start()
{
    this->_server->start();
    this->_threadPool->start(std::thread::hardware_concurrency() << 1);
    this->_loop->loop();
    return true;
}

void ProxyServer::quit()
{
    this->_loop->quit();
    this->_threadPool.reset();
    this->_server.reset();
}

void ProxyServer::onMessage(const mg::TcpConnectionPointer &link, mg::Buffer *b, mg::TimeStamp c)
{
    while (1)
    {
        std::string data;
        if (!mg::TcpPacketParser::get().reveive(link, data))
            break;
        auto callback = HandlerMap::get().getCallBack(link, data);
        if (callback)
            this->_threadPool->append(std::move(callback));
        else
            LOG_ERROR("{} parse error", link->name());
    }
}