#include "business-task.h"
#include "file-info.h"
#include "../src/tcp-packet-parser.h"
#include "../src/json-extract.h"
#include "../src/mysql.h"
#include "../src/mysql-connection-pool.h"
#include "../src/macros.h"
#include "../protocal/protocal-session.h"

#include <crypt.h>
using namespace Protocal;

bool BusinessTask::parse(const mg::TcpConnectionPointer &connection, Protocal::SessionCommand &data)
{
    json js = json::parse(data.unserialize());
    if (!js.contains("connection-name"))
        return false;

    bool valid = true;

    switch (TO_ENUM(SessionType, data.type))
    {
    case SessionType::LOGIN:
        valid = login(connection, js);
        break;
    case SessionType::REGIST:
        valid = regist(connection, js);
        break;
    case SessionType::UPLOAD:
        valid = upload(connection, js);
        break;
    }

    return valid;
}

bool BusinessTask::login(TCPCONNECTION &con, const json &jsData)
{
    std::string name, password;
    if (!mg::JsonExtract::extract(jsData, "name", name, mg::JsonExtract::STRING))
        return false;
    if (!mg::JsonExtract::extract(jsData, "password", password, mg::JsonExtract::STRING))
        return false;

    if (name.empty() || password.empty())
        return false;

    std::shared_ptr<mg::Mysql> sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
    {
        LOG_ERROR("get mysql handle failed");
        return false;
    }

    json ret;
    ret["connection-name"] = jsData["connection-name"];

    std::ostringstream os;
    os << "select username, passwd, salt from user_info where ";
    os << "username = \'" << name << "\'";
    if (!sql->query(os.str()) || !sql->next())
    {
        ret["status"] = "failed";
        ret["detail"] = "user not exit";
        mg::TcpPacketParser::get().send(con, SessionCommand().serialize(ret.dump()));
        return false;
    }

    std::string salt = sql->getData("salt");
    struct crypt_data cryptData;
    memset(&cryptData, 0, sizeof(cryptData));
    char *crypt = ::crypt_r(password.c_str(), salt.c_str(), &cryptData);
    if (crypt == nullptr)
        return false;

    std::string cryptPassword(::strrchr(crypt, '$') + 1);
    if (cryptPassword != sql->getData("passwd"))
    {
        ret["status"] = "failed";
        ret["detail"] = "password error";
        mg::TcpPacketParser::get().send(con, SessionCommand().serialize(ret.dump()));
        return false;
    }

    ret["con-state"] = TO_UNDERLYING(ConState::VERIFY);
    ret["status"] = "success";
    mg::TcpPacketParser::get().send(con, SessionCommand().serialize(ret.dump()));
    return true;
}

bool BusinessTask::regist(TCPCONNECTION &con, const json &jsData)
{
    std::string salt = "$y$j9T$byV0Zo35gBDQJtKsEx.XR/";
    std::string name = jsData.value("name", "");
    std::string password = jsData.value("password", "");

    struct crypt_data cryptData;
    memset(&cryptData, 0, sizeof(cryptData));
    char *crypt = ::crypt_r(password.c_str(), salt.c_str(), &cryptData);

    static mg::DataField field[] =
        {
            {"username", mg::DataType::DB_STRING, 33},
            {"salt", mg::DataType::DB_STRING, 129},
            {"passwd", mg::DataType::DB_STRING, 1024},
            {nullptr, mg::DataType::DB_INVALID, 0} // end of definition
        };

    struct Filed
    {
        char username[33];
        char salt[129];
        char password[1024];
    } __attribute__((packed));

    Filed data;
    ::strncpy(data.salt, salt.c_str(), sizeof(data.salt) - 1);
    ::strncpy(data.password, ::strrchr(crypt, '$') + 1, sizeof(data.password) - 1);
    ::strncpy(data.username, name.c_str(), sizeof(data.username) - 1);

    std::shared_ptr<mg::Mysql> sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
    {
        LOG_ERROR("get mysql handle failed");
        return false;
    }
    sql->insert("user_info", field, (char *)&data);
    return true;
}

bool BusinessTask::upload(TCPCONNECTION &con, const json &jsData)
{
    uint16_t state = 0;
    if (!mg::JsonExtract::extract(jsData, "con-state", state, mg::JsonExtract::INT))
        return false;
    if (state != TO_UNDERLYING(ConState::VERIFY))
        return false;

    int size = 0;
    std::string filename, hash;
    if (!mg::JsonExtract::extract(jsData, "filename", filename, mg::JsonExtract::STRING))
        return false;

    if (!fileMemo.count(filename))
        fileMemo[filename] = std::make_unique<FileInfo>(filename, FileInfo::FILEMODE::WRITE);
    auto &fileInfo = fileMemo[filename];

    switch (fileInfo->getFileStatus())
    {
    case FileInfo::FILESTATUS::WAITING_INFO:
    {
        if (!waitFileInfo(filename, jsData))
            return false;
        break;
    }

    case FileInfo::FILESTATUS::UPLOADING:
    {
        if (!uploading(filename, jsData))
            return false;
        if (fileInfo->isCompleted())
        {
            json js;
            fileInfo->setFileStatus(FileInfo::FILESTATUS::COMPLETED);
            js["connection-name"] = jsData["connection-name"];
            js["filename"] = filename;
            js["status"] = "success";
            mg::TcpPacketParser::get().send(con, SessionCommand().serialize(js.dump()));
        }
        break;
    }

    case FileInfo::FILESTATUS::COMPLETED:
        return true;
    }

    json ret;
    ret["connection-name"] = jsData["connection-name"];
    ret["status"] = "success";
    mg::TcpPacketParser::get().send(con, SessionCommand().serialize(ret.dump()));
    return true;
}

bool BusinessTask::waitFileInfo(const std::string &filename, const json &jsData)
{
    auto &fileInfo = fileMemo[filename];
    int size = 0;
    std::string hash;
    if (!mg::JsonExtract::extract(jsData, "hash", hash, mg::JsonExtract::STRING))
        return false;
    if (!mg::JsonExtract::extract(jsData, "size", size, mg::JsonExtract::INT))
        return false;
    fileInfo->setFileHash(hash);
    fileInfo->setFileSize(size);
    fileInfo->setFileStatus(FileInfo::FILESTATUS::UPLOADING);
    return true;
}

bool BusinessTask::uploading(const std::string &filename, const json &jsData)
{
    auto &fileInfo = fileMemo[filename];
    int chunkIndex = 0;
    if (!mg::JsonExtract::extract(jsData, "chunkIndex", chunkIndex, mg::JsonExtract::INT))
        return false;
    std::string data;
    if (!mg::JsonExtract::extract(jsData, "data", data, mg::JsonExtract::BINARY))
        return false;
    return fileInfo->write(chunkIndex, data) == data.size();
}