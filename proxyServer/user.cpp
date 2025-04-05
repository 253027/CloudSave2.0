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
        list.push_back(sql->getData<uint32_t>("peer"));

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

    info.set_user_id(sql->getData<uint32_t>("id"));
    info.set_user_gender(sql->getData<uint8_t>("sex"));
    info.set_user_nick_name(sql->getData("nick"));
    info.set_email(sql->getData("email"));
    info.set_user_real_name(sql->getData("name"));
    info.set_user_tel(sql->getData("telphone"));
    info.set_avatar_url(sql->getData("avatar"));
    return true;
}
