#include "file-server.h"
#include "../src/json.hpp"
#include "../src/log.h"

#include <fstream>

using json = nlohmann::json;

FileServer::FileServer()
{
    ;
}

FileServer::~FileServer()
{
    ;
}

bool FileServer::initial()
{
    json js;
    std::ifstream file("./FileServer/config.json");
    if (!file.is_open())
    {
        LOG_ERROR("FileServer read config failed");
        return false;
    }
    try
    {
        file >> js;
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("config.json parse error: {}", e.what());
        return false;
    }

    _loop.reset(new mg::EventLoop("FileServerLoop"));
    _server.reset(new mg::TcpServer(_loop.get(), mg::InternetAddress(js.value("port", 0)), "FileServer"));
    _server->setMessageCallback(std::bind(&FileServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _server->setThreadNums(js.value("threadNums", 0));
    return true;
}

void FileServer::start()
{
    _server->start();
    _loop->loop();
}

void FileServer::stop()
{
    _loop->quit();
    _server.reset();
}

void FileServer::onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c)
{
    ;
}
