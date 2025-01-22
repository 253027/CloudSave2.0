#include "file-server.h"
#include "file-info.h"
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
    std::ifstream file("./FileServer/config.json");
    if (!file.is_open())
    {
        LOG_ERROR("FileServer read config failed");
        return false;
    }
    try
    {
        file >> this->_config;
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("config.json parse error: {}", e.what());
        return false;
    }

    _loop.reset(new mg::EventLoop("FileServerLoop"));
    _server.reset(new mg::TcpServer(_loop.get(), mg::InternetAddress(this->_config.value("port", 0)), "FileServer"));
    _server->setMessageCallback(std::bind(&FileServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _server->setThreadNums(std::max(static_cast<unsigned int>(this->_config.value("threadNums", 0)), std::thread::hardware_concurrency()));
    _calcPool.reset(new mg::EventLoopThreadPool(_loop.get(), "FileServerCalcLoop"));
    _calcPool->setThreadNums(2); // 默认设置2个

    this->regist();
    this->loadSource();
    return true;
}

void FileServer::start()
{
    _server->start();
    _calcPool->start();
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
    if (TO_ENUM(FILESTATE, a->getUserConnectionState()) != FILESTATE::VERIFY)
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
    if (!request.hasHeader("x-chunk-number") ||
        !request.hasHeader("x-file-name"))
        return false;

    auto a = request.getConnection();
    if (TO_ENUM(FILESTATE, a->getUserConnectionState()) != FILESTATE::VERIFY)
        return false;

    auto it_file = fileInfoMemo.find(a->name());
    auto it = it_file->second.find(request.getHeader("x-file-name"));
    if (it == it_file->second.end())
        return false;
    std::shared_ptr<FileInfo> file = it->second; // get file handle

    if (file->getFileStatus() != FileInfo::FILESTATUS::UPLOADING)
        return false;

    int chunkIndex = 0;
    try
    {
        chunkIndex = std::stoi(request.getHeader("x-chunk-number"));
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("chunk number parse error {}, which is {}", e.what(), request.getHeader("x-chunk-number"));
        return false;
    }

    if (file->write(chunkIndex, request.body()) != request.body().size())
        return false;

    if (file->getOwnerLoop() == nullptr)
        file->setOwnerLoop(_calcPool->getNextLoop());

    file->getOwnerLoop()->push(std::bind(&FileInfo::update, file,
                                         std::vector<unsigned char>(request.body().begin(), request.body().end())));

    if (file->isCompleted())
    {
        file->getOwnerLoop()->push(std::bind(&FileServer::judgeFileMD5, this, std::move(file), std::move(a), false));
        return true;
    }

    mg::HttpResponse response;
    response.setStatus(200);
    mg::HttpPacketParser::get().send(a, response);
    return true;
}

bool FileServer::waitFileInfo(const mg::HttpRequest &request)
{
    json js;
    auto a = request.getConnection();
    if (TO_ENUM(FILESTATE, a->getUserConnectionState()) != FILESTATE::VERIFY)
        return false;

    mg::HttpResponse response;
    if (!mg::JsonExtract::parse(js, request.body()))
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

    // insert file information into fileInfoMemo
    auto file = std::make_shared<FileInfo>(this->_config.value("filepath", "./") + filename, md5, size, FileInfo::WRITE);
    file->setFileStatus(FileInfo::FILESTATUS::UPLOADING);
    fileInfoMemo[a->name()][filename] = file;

    js.clear();
    js["chunk_size"] = file->getChunkSize();
    LOG_INFO("{}: filename:[{}] md5:[{}] size:[{}]", a->name(), filename, md5, size);

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
    if (TO_ENUM(FILESTATE, state) != FILESTATE::UNVERIFY)
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
        LOG_ERROR("{} {} {}", a->name(), name, password);
        goto end;
    }

    response.setStatus(302);
    response.setHeader("Location", "/upload.html");
    mg::HttpPacketParser::get().send(a, response);
    a->setUserConnectionState(TO_UNDERLYING(FILESTATE::VERIFY));
    return true;

end:
    response.setStatus(400);
    response.setHeader("Content-Type", "application/json");
    response.setBody(js.dump());
    mg::HttpPacketParser::get().send(a, response);
    return false;
}

void FileServer::judgeFileMD5(std::shared_ptr<FileInfo> &file, mg::TcpConnectionPointer &connection, bool needCalc)
{
    mg::HttpResponse response;
    if (file->verify(needCalc))
    {
        response.setStatus(200);
        json js;
        js["status"] = "success";
        response.setHeader("Content-Type", "application/json");
        response.setBody(js.dump());
    }
    else
        response.setStatus(400);
    mg::HttpPacketParser::get().send(connection, response);
}