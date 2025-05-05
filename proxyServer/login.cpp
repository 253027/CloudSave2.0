#include "login.h"
#include "../src/base/mysql-connection-pool.h"

#include <sstream>

bool Login::doLogin(const std::string &userName, const std::string &password, IM::BaseDefine::UserInformation &info)
{
    auto connection = mg::MysqlConnectionPool::get().getHandle();
    if (!connection)
        return false;

    std::array<std::string, 0> column;
    auto data = std::make_tuple(userName);
    if (!connection->select("UserInformation", column, "name=?", data))
        return false;

    if (connection->next())
    {
        std::string salt = connection->getData("salt");

        // TODO: calculate the md5 of password
        if (connection->getData("password") != password)
            return false;

        info.set_user_id(connection->getData("id"));
        info.set_user_gender(connection->getData("sex"));
        {
            std::string nick = connection->getData("nick");
            info.set_user_nick_name(nick);
        }
        {
            std::string email = connection->getData("email");
            info.set_email(email);
        }
        {
            std::string telphone = connection->getData("telphone");
            info.set_user_tel(telphone);
        }
        {
            std::string avatar = connection->getData("avatar");
            info.set_avatar_url(avatar);
        }
        return true;
    }
    return false;
}