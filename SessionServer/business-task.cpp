#include "business-task.h"
#include "../src/tcp-packet-parser.h"
#include "../src/json.hpp"
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
    }

    return valid;
}

bool BusinessTask::login(TCPCONNECTION &con, const json &jsData)
{
    if (!jsData.contains("name") || !jsData["name"].is_string())
        return false;
    if (!jsData.contains("password") || !jsData["password"].is_string())
        return false;

    std::string name = jsData["name"];
    std::string password = jsData["password"];
    if (name.empty() || password.empty())
        return false;

    std::shared_ptr<mg::Mysql> sql = mg::MysqlConnectionPool::getMe().getHandle();
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
        mg::TcpPacketParser::getMe().send(con, SessionCommand().serialize(ret.dump()));
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
        mg::TcpPacketParser::getMe().send(con, SessionCommand().serialize(ret.dump()));
        return false;
    }

    ret["status"] = "success";
    mg::TcpPacketParser::getMe().send(con, SessionCommand().serialize(ret.dump()));
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

    std::shared_ptr<mg::Mysql> sql = mg::MysqlConnectionPool::getMe().getHandle();
    if (!sql)
    {
        LOG_ERROR("get mysql handle failed");
        return false;
    }
    sql->insert("user_info", field, (char *)&data);
    return true;
}
