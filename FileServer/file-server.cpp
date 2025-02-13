#include "file-server.h"
#include "file-info.h"
#include "../src/log.h"
#include "../src/http-method-call.h"
#include "../src/json-extract.h"
#include "../src/macros.h"

#include <fstream>
#include <sstream>

using json = nlohmann::json;

struct UserInfo
{
    UserInfo() {}

    UserInfo(int id, const std::string &name) : id(id), name(name)
    {
        ;
    }

    int id;
    std::string name;
};

thread_local std::unordered_map<std::string, UserInfo> connectionInfo;

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
    _server->setConnectionCallback(std::bind(&FileServer::onConnectionStateChanged, this, std::placeholders::_1));
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
    mg::HttpMethodCall::get().regist2("GET", "/download/.*", std::bind(&FileServer::download, this, std::placeholders::_1));
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

void FileServer::onConnectionStateChanged(const mg::TcpConnectionPointer &connection)
{
    if (!connection->connected())
    {
        connectionInfo.erase(connection->name());
        fileInfoMemo.erase(connection->name());
        LOG_DEBUG("{} disconnected", connection->name());
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
        fileInfoMemo[a->name()].erase(file->fileName());
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

    uint64_t size = 0;
    std::string filename, md5;
    if (!mg::JsonExtract::extract(js, "fileSize", size, mg::JsonExtract::INT) ||
        !mg::JsonExtract::extract(js, "fileName", filename, mg::JsonExtract::STRING) ||
        !mg::JsonExtract::extract(js, "fileMD5", md5, mg::JsonExtract::STRING))
        return false;

    // insert file information into fileInfoMemo
    auto file = std::make_shared<FileInfo>(md5, md5, size, FileInfo::WRITE, this->_config.value("filepath", "./"));
    file->setFileStatus(FileInfo::FILESTATUS::UPLOADING);
    file->setFileName(filename);
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
    auto it = connectionInfo.find(a->name());
    if (it == connectionInfo.end())
    {
        LOG_ERROR(" can not find {}", a->name());
        return false;
    }
    auto &user = it->second;

    std::stringstream query;
    query << "SELECT `file_name`, `file_size` FROM `Files` WHERE `user_id` = " << user.id;

    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql || !sql->query(query.str()))
    {
        LOG_ERROR("query failed");
        return false;
    }

    while (sql->next())
    {
        js.push_back({{"name", sql->getData("file_name")}, {"size", std::stoull(sql->getData("file_size"))}});
    }

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
    query << "SELECT password, id FROM UserInfo WHERE `username` = \'" << name << "\';";
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

    if (sql->getData("password") != password)
    {
        js["status"] = "failed";
        js["message"] = "password error";
        LOG_ERROR("{} {} {}", a->name(), name, password);
        goto end;
    }
    connectionInfo[a->name()] = UserInfo(std::stoi(sql->getData("id")), name);

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
        connection->getLoop()->push(std::bind(&FileServer::updateDataBase, this, file, connection->name()));
    }
    else
        response.setStatus(400);
    mg::HttpPacketParser::get().send(connection, response);
}

void FileServer::updateDataBase(std::shared_ptr<FileInfo> &file, const std::string &name)
{
    auto it = connectionInfo.find(name);
    if (it == connectionInfo.end())
    {
        LOG_ERROR("file {} can not find {}", file->fileName(), name);
        return;
    }
    auto &user = it->second;

    std::shared_ptr<mg::Mysql> sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
    {
        LOG_ERROR("get mysql handle failed");
        return;
    }

    std::stringstream is;
    is << "INSERT INTO `Files` (`file_name`, `file_size`, `hash_value`, `user_id`) VALUES ('"
       << file->getName() << "', '" << file->getFileSize() << "', '"
       << file->getFileHash() << "', " << user.id << ");";

    if (!sql->insert(is.str()))
        LOG_ERROR("insert fileinfo error: {}", is.str());
    file->setFileStatus(FileInfo::FILESTATUS::COMPLETED);
}

bool FileServer::download(const mg::HttpRequest &request)
{
    auto connection = request.getConnection();
    if (TO_ENUM(FILESTATE, connection->getUserConnectionState()) != FILESTATE::VERIFY)
        return false;

    auto &userInfo = connectionInfo[connection->name()];

    auto it = request.path().find("/download/");
    if (it == std::string::npos)
        return false;
    std::string filename = request.path().substr(it + 10);

    // if download file not in fileInfoMemo
    {
        auto it_file = fileInfoMemo[connection->name()].find(filename);
        if (it_file == fileInfoMemo[connection->name()].end())
        {
            std::stringstream query;
            query << "SELECT hash_value, file_size FROM `Files`"
                  << "LEFT JOIN `UserInfo` ON Files.user_id = UserInfo.id WHERE username = \'"
                  << userInfo.name << "\' and file_name = \'" << filename << "\';";

            auto sql = mg::MysqlConnectionPool::get().getHandle();
            if (!sql || !sql->query(query.str()))
            {
                LOG_ERROR("{} get download file info failed", connection->name());
                return false;
            }

            if (!sql->next())
                return false;

            auto file = std::make_shared<FileInfo>(sql->getData("hash_value"), FileInfo::READ, this->_config.value("filepath", "./"));
            fileInfoMemo[connection->name()][filename] = file;
        }
    }
    auto file = fileInfoMemo[connection->name()][filename];

    if (request.hasHeader("range")) // request ranges bytes
    {
        // std::regex pattern("bytes=(\d+)-(\d+)");
        // std::smatch result;
        // if (!std::regex_search(request.getHeader("range"), result, pattern))
        //     return false;

        // std::string data =
    }
    else
    {
        mg::HttpResponse ret;
        ret.setStatus(200);
        ret.setHeader("Content-Disposition", "attachment; filename=" + filename);
        ret.setHeader("Connection", "keep-alive");
        ret.setHeader("Transfer-Encoding", "chunked");
        ret.setHeader("Accept-Ranges", "bytes");
        ret.setHeader("Content-Type", "application/octet-stream");
        connection->send(ret.dumpHead() + "\r\n");
        connection->getLoop()->push(std::bind(&FileServer::streamSend, FileServer::getInstance(), 0, std::move(connection), std::move(filename)));
    }

    return true;
}

void FileServer::streamSend(int index, const mg::TcpConnectionPointer &connection, const std::string &filename)
{
    auto it_connection = fileInfoMemo.find(connection->name());
    if (it_connection == fileInfoMemo.end())
        return;
    auto it_file = it_connection->second.find(filename);
    if (it_file == it_connection->second.end())
        return;
    auto file = it_file->second;

    std::string data = file->read(index * file->getChunkSize(), file->getChunkSize());
    std::stringstream range;
    range << std::hex << data.size() << "\r\n"
          << data << "\r\n";

    connection->send(range.str());

    if (index + 1 >= file->getChunkNums())
    {
        connection->send("0\r\n\r\n");
        return;
    }

    connection->getLoop()->push(std::bind(&FileServer::streamSend, FileServer::getInstance(),
                                          index + 1, std::move(connection), std::move(filename)));
}
