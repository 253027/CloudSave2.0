#include "business-task.h"
#include "../ServerSDK/tcp-packet-parser.h"
#include "../ServerSDK/json.hpp"
#include "../ServerSDK/mysql.h"
#include "../ServerSDK/mysql-connection-pool.h"
#include "../ServerSDK/macros.h"

#include <crypt.h>

void BusinessTask::parse(const mg::TcpConnectionPointer &connection, const std::string &data)
{
    LOG_DEBUG("data: {}", data);
    json js = json::parse(data);

    MethodType type = TO_ENUM(MethodType, js["type"]);
    switch (type)
    {
    case MethodType::LOGIN:
        login(connection, js);
        break;
    case MethodType::REGIST:
        regist(connection, js);
        break;
    default:
        break;
    }

    mg::TcpPacketParser::getMe().send(connection, data);
}

void BusinessTask::login(TCPCONNECTION &con, const json &jsData)
{
    ConnectionState state = TO_ENUM(ConnectionState, con->getUserConnectionState());
    if (state != ConnectionState::UNVERIFY)
        return;

    std::string name = "name";
    std::string password = "password";
    if (!jsData.contains(name) || !jsData.contains(password))
        return;

    name = jsData[name];
    password = jsData[password];
    if (name.empty() || password.empty())
        return;

    std::shared_ptr<mg::Mysql> sql = mg::MysqlConnectionPool::getMe().getHandle();
    if (!sql)
    {
        LOG_ERROR("get mysql handle failed");
        return;
    }

    std::ostringstream os;
    os << "select username, passwd, salt from user_info where ";
    os << "username = \'" << name << "\'";
    sql->query(os.str());
    if (!sql->next())
        return;

    std::string salt = sql->getData("salt");
    struct crypt_data cryptData;
    memset(&cryptData, 0, sizeof(cryptData));
    char *crypt = ::crypt_r(password.c_str(), salt.c_str(), &cryptData);
    if (crypt == nullptr)
        return;

    std::string cryptPassword(::strrchr(crypt, '$') + 1);
    if (cryptPassword != sql->getData("passwd"))
        return;

    con->setUserConnectionState(TO_UNDERLYING(ConnectionState::VERIFY));
}

void BusinessTask::regist(TCPCONNECTION &con, const json &jsData)
{
    if (TO_ENUM(ConnectionState, con->getUserConnectionState()) != ConnectionState::UNVERIFY)
        return;

    std::string salt = "$y$j9T$byV0Zo35gBDQJtKsEx.XR/";
    std::string password = jsData["password"];
    std::string name = jsData["name"];

    struct crypt_data cryptData;
    memset(&cryptData, 0, sizeof(cryptData));
    char *crypt = ::crypt_r(password.c_str(), salt.c_str(), &cryptData);

    mg::DataField field[] =
        {
            {"username", mg::DataType::DB_STRING, 33},
            {"salt", mg::DataType::DB_STRING, 129},
            {"passwd", mg::DataType::DB_STRING, 1024},
            {nullptr, mg::DataType::DB_INVALID, 0} //
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
        return;
    }
    sql->insert("user_info", field, (char *)&data);
}
