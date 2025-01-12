#include "file-server.h"
#include "../src/json.hpp"
#include "../src/log.h"
#include "../src/http-method-call.h"
#include "../src/json-extract.h"
#include "../src/macros.h"

#include <fstream>
#include <sstream>

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

    this->regist();
    this->loadSource();
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
    while (1)
    {
        mg::HttpRequest request(a);
        if (!mg::HttpPacketParser::get().reveive(a, request))
            break;
        if (!mg::HttpMethodCall::get().exec(request))
        {
            mg::HttpResponse req;
            req.setStatus(400);
            mg::HttpPacketParser::get().send(a, req);
        }
    }
}

void FileServer::regist()
{
    mg::HttpMethodCall::get().regist("GET", "/index.html", std::bind(&FileServer::main, this, std::placeholders::_1));
    mg::HttpMethodCall::get().regist("GET", "/upload.html", std::bind(&FileServer::uploadPage, this, std::placeholders::_1));
    mg::HttpMethodCall::get().regist("GET", "/file-list", std::bind(&FileServer::fileInfo, this, std::placeholders::_1));
    mg::HttpMethodCall::get().regist("POST", "/upload", std::bind(&FileServer::upload, this, std::placeholders::_1));
    mg::HttpMethodCall::get().regist("POST", "/login", std::bind(&FileServer::login, this, std::placeholders::_1));
    mg::HttpMethodCall::get().regist("POST", "/upload/file-info", std::bind(&FileServer::waitFileInfo, this, std::placeholders::_1));
}

void FileServer::loadSource()
{
    std::fstream file("./FileServer/source/index.html");
    if (file.is_open())
    {
        this->_indexContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
    }

    file.open("./FileServer/source/upload.html");
    if (file.is_open())
    {
        this->_uploadIndexContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
    }
}

bool FileServer::main(const mg::HttpRequest &request)
{
    auto a = request.getConnection();
    mg::HttpResponse response;
    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");

    if (!_indexContent.empty())
        response.setBody(_indexContent);
    else
        response.setBody("<html>Hello World!</html>");
    mg::HttpPacketParser::get().send(a, response);
    return true;
}

bool FileServer::uploadPage(const mg::HttpRequest &request)
{
    auto a = request.getConnection();
    if (TO_ENUM(FILESTATE, a->getUserConnectionState()) == FILESTATE::FILE_LOGIN)
        return false;

    mg::HttpResponse response;
    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");
    if (!_indexContent.empty())
        response.setBody(_uploadIndexContent);
    else
        response.setBody("<html>Hello World!</html>");
    mg::HttpPacketParser::get().send(a, response);
    return true;
}

bool FileServer::upload(const mg::HttpRequest &request)
{
    auto a = request.getConnection();
    if (TO_ENUM(FILESTATE, a->getUserConnectionState()) != FILESTATE::FILE_UPLOAD)
        return false;
    mg::HttpResponse response;
    response.setStatus(200);
    response.setHeader("Location", "/upload.html");
    LOG_DEBUG("content size: {}", request.body().size());
    mg::HttpPacketParser::get().send(a, response);
    return true;
}

bool FileServer::waitFileInfo(const mg::HttpRequest &request)
{
    json js;
    auto a = request.getConnection();
    if (TO_ENUM(FILESTATE, a->getUserConnectionState()) != FILESTATE::FILE_WAIT_INFO)
        return false;
    a->setUserConnectionState(TO_UNDERLYING(FILESTATE::FILE_UPLOAD));
    mg::HttpResponse response;

    if (mg::JsonExtract::parse(js, request.body()))
    {
        LOG_ERROR("fileInfo parse error {}", a->name());
        return false;
    }

    int size = 0;
    std::string filename, md5;
    if (!mg::JsonExtract::extract(js, "fileSize", size, mg::JsonExtract::INT) ||
        !mg::JsonExtract::extract(js, "fileName", filename, mg::JsonExtract::STRING) ||
        !mg::JsonExtract::extract(js, "fileMD5", md5, mg::JsonExtract::STRING))
        return false;

    LOG_INFO("filename:[{}] md5:[{}] size:[{}]", filename, md5, size);
    const int ChunkSize = 8 * 1024 * 1024; // 分块大小8M
    js.clear();
    js["chunk_size"] = ChunkSize;

    response.setStatus(200);
    response.setHeader("Content-Type", "application/json");
    response.setBody(js.dump());
    mg::HttpPacketParser::get().send(a, response);
    return true;
}

bool FileServer::fileInfo(const mg::HttpRequest &request)
{
    json js;
    auto a = request.getConnection();
    mg::HttpResponse response;

    // 这里先手动填充写文件信息，待后面修改
    js.push_back({{"name", "test.txt"}, {"size", 1024}});
    js.push_back({{"name", "test2.txt"}, {"size", 2048}});
    js.push_back({{"name", "test3.txt"}, {"size", 4096}});

    response.setStatus(200);
    response.setHeader("Content-Type", "application/json");
    response.setBody(js.dump());
    mg::HttpPacketParser::get().send(a, response);
    return true;
}

bool FileServer::login(const mg::HttpRequest &request)
{
    auto a = request.getConnection();
    auto state = a->getUserConnectionState();
    if (TO_ENUM(FILESTATE, state) != FILESTATE::FILE_LOGIN)
    {
        mg::HttpResponse response; // redirect the url
        response.setStatus(302);
        response.setHeader("Location", "/upload.html");
        mg::HttpPacketParser::get().send(a, response);
        return true;
    }

    json js;
    bool valid = true;
    std::tie(valid, js) = mg::JsonExtract::parse(request.body());
    if (!valid)
        return false;

    mg::HttpResponse response;

    std::string name, password;
    if (!mg::JsonExtract::extract(js, "username", name, mg::JsonExtract::STRING))
        return false;
    if (!mg::JsonExtract::extract(js, "password", password, mg::JsonExtract::STRING))
        return false;

    if (name.empty() || password.empty())
        return false;

    std::shared_ptr<mg::Mysql> sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
    {
        LOG_ERROR("get mysql handle failed");
        return false;
    }

    std::stringstream query;
    query << "SELECT passwd FROM user_info WHERE `username` = \'" << name << "\';";
    if (!sql->query(query.str()))
    {
        LOG_ERROR("query failed {}", a->name());
        return false;
    }

    js.clear();
    if (!sql->next())
    {
        js["status"] = "failed";
        js["message"] = "user not exit";
        goto end;
    }

    if (sql->getData("passwd") != password)
    {
        js["status"] = "failed";
        js["message"] = "password error";
        goto end;
    }

    response.setStatus(302);
    response.setHeader("Location", "/upload.html");
    mg::HttpPacketParser::get().send(a, response);
    a->setUserConnectionState(TO_UNDERLYING(FILESTATE::FILE_WAIT_INFO));
    return true;

end:
    response.setStatus(400);
    response.setHeader("Content-Type", "application/json");
    response.setBody(js.dump());
    mg::HttpPacketParser::get().send(a, response);
    return false;
}