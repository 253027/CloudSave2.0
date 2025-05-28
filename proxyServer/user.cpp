#include "user.h"
#include "../src/base/mysql-connection-pool.h"

bool User::getFriendsList(uint32_t userId, std::vector<uint32_t> &list, uint32_t lastUpdateTime)
{
    auto handle = mg::MysqlConnectionPool::get().getHandle();
    if (!handle)
        return false;

    if (lastUpdateTime)
    {
        std::string sql = "SELECT `peer` FROM `RelationShip` WHERE `user`=? AND `updateTime`<=? AND `stauts`=0";
        auto data = std::make_tuple(userId, lastUpdateTime);
        if (!handle->select(sql, data))
            return false;
    }
    else
    {
        std::string sql = "SELECT `peer` FROM `RelationShip` WHERE `user`=? AND `status`=0";
        auto data = std::make_tuple(userId);
        if (!handle->select(sql, data))
            return false;
    }

    while (handle->next())
        list.push_back(handle->getData("peer"));

    return true;
}

bool User::getFriendsInfo(uint32_t userId, IM::DataStruct::UserInformation &info)
{
    auto handle = mg::MysqlConnectionPool::get().getHandle();
    if (!handle)
        return false;

    std::string sql = "SELECT * FROM `UserInformation` WHERE `id`=?";
    auto data = std::make_tuple(userId);
    if (!handle->select(sql, data))
        return false;

    if (!handle->next())
        return false;

    info.set_user_id(handle->getData("id"));
    info.set_user_gender(handle->getData("sex"));
    {
        std::string nick = handle->getData("nick");
        info.set_user_nick_name(nick);
    }
    {
        std::string email = handle->getData("email");
        info.set_email(email);
    }
    {
        std::string name = handle->getData("name");
        info.set_user_real_name(name);
    }
    {
        std::string telphone = handle->getData("telphone");
        info.set_user_tel(telphone);
    }
    {
        std::string avatar = handle->getData("avatar");
        info.set_avatar_url(avatar);
    }

    return true;
}
