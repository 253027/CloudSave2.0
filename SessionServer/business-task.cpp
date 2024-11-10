#include "business-task.h"
#include "../ServerSDK/tcp-packet-parser.h"
#include "../ServerSDK/json.hpp"
#include "../ServerSDK/mysql.h"
#include "../ServerSDK/mysql-connection-pool.h"
#include "../ServerSDK/macros.h"

#include <crypt.h>

void BusinessTask::parse(const mg::TcpConnectionPointer &connection, const std::string &data)
{
    json js = json::parse(data);

    MethodType type = TO_ENUM(MethodType, js.value("type", 0));
    switch (type)
    {
    case MethodType::LOGIN:
        login(connection, js);
        break;
    case MethodType::REGIST:
        regist(connection, js);
        break;
    }

    mg::TcpPacketParser::getMe().send(connection, data);
}

void BusinessTask::login(TCPCONNECTION &con, const json &jsData)
{
    ConnectionState state = TO_ENUM(ConnectionState, con->getUserConnectionState());
    if (state != ConnectionState::UNVERIFY)
        return;

    std::string name = jsData.value("name", "");
    std::string password = jsData.value("password", "");
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
    if (!sql->query(os.str()) || !sql->next())
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
    json retData;
    retData["type"] = TO_UNDERLYING(MethodType::LOGIN);
    retData["status"] = "success";
    mg::TcpPacketParser::getMe().send(con, retData.dump());
}

void BusinessTask::regist(TCPCONNECTION &con, const json &jsData)
{
    if (TO_ENUM(ConnectionState, con->getUserConnectionState()) != ConnectionState::UNVERIFY)
        return;

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
        return;
    }
    sql->insert("user_info", field, (char *)&data);
}
