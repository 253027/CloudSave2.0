#include "user.h"
#include "../src/base/mysql-connection-pool.h"

bool User::getFriendsList(uint32_t userId, std::vector<uint32_t> &list, uint32_t lastUpdateTime)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return false;

    std::array<std::string, 3> column = {"user", "peer", "updateTime"};
    if (lastUpdateTime)
    {
        auto data = std::make_tuple(userId, lastUpdateTime);
        if (!sql->select("RelationShip", column, "user=? AND updateTime <=? AND status=0", data))
            return false;
    }
    else
    {
        auto data = std::make_tuple(userId);
        if (!sql->select("RelationShip", column, "user=? AND status=0", data))
            return false;
    }

    while (sql->next())
        list.push_back(sql->getData("peer"));

    return true;
}

bool User::getFriendsInfo(uint32_t userId, IM::BaseDefine::UserInformation &info)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return false;

    std::array<std::string, 0> column;
    auto data = std::make_tuple(userId);
    if (!sql->select("UserInformation", column, "id=?", data))
        return false;

    if (!sql->next())
        return false;

    info.set_user_id(sql->getData("id"));
    info.set_user_gender(sql->getData("sex"));
    {
        std::string nick = sql->getData("nick");
        info.set_user_nick_name(nick);
    }
    {
        std::string email = sql->getData("email");
        info.set_email(email);
    }
    {
        std::string name = sql->getData("name");
        info.set_user_real_name(name);
    }
    {
        std::string telphone = sql->getData("telphone");
        info.set_user_tel(telphone);
    }
    {
        std::string avatar = sql->getData("avatar");
        info.set_avatar_url(avatar);
    }

    return true;
}
