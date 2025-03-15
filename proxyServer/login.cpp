#include "login.h"
#include "../src/base/mysql-connection-pool.h"

#include <sstream>

bool Login::doLogin(const std::string &userName, const std::string &password, IM::BaseDefine::UserInformation &info)
{
    auto connection = mg::MysqlConnectionPool::get().getHandle();
    if (!connection)
        return false;

    std::stringstream query;
    query << "SELECT * FROM `UserInformation` WHERE `name`='" << userName << "'";
    if (!connection->query(query.str()))
        return false;

    while (connection->next())
    {
        std::string salt = connection->getData("salt");

        // TODO: calculate the md5 of password
        if (connection->getData("password") != password)
            return false;

        info.set_user_id(std::stoi(connection->getData("id")));
        info.set_user_gender(std::stoi(connection->getData("sex")));
        info.set_user_nick_name(connection->getData("nick"));
        info.set_email(connection->getData("email"));
        info.set_user_tel(connection->getData("telphone"));
        info.set_avatar_url(connection->getData("avatar"));
    }

    return true;
}