#include "session.h"
#include "../src/base/mysql-connection-pool.h"
#include "../src/base/time-stamp.h"

#include <sstream>

uint32_t Session::getSession(uint32_t from, uint32_t to, uint32_t type, bool tomb)
{
    auto handle = mg::MysqlConnectionPool::get().getHandle();
    if (!handle)
        return 0;

    if (!tomb)
    {
        std::string sql = "SELECT `id` FROM `Session` WHERE `user`=? AND `peer`=? AND `type`=? AND `status`=0";
        auto data = std::make_tuple(from, to, type);
        if (!handle->select(sql, data))
            return 0;
    }
    else
    {
        std::string sql = "SELECT `id` FROM `Session` WHERE `user`=? AND `peer`=?";
        auto data = std::make_tuple(from, to);
        if (!handle->select(sql, data))
            return 0;
    }

    if (handle->next())
        return handle->getData("id");

    return 0;
}

uint32_t Session::addSession(uint32_t from, uint32_t to, uint32_t type)
{
    auto handle = mg::MysqlConnectionPool::get().getHandle();
    if (!handle)
        return 0;

    uint32_t id = this->getSession(from, to, type, true);

    if (id)
    {
        std::string sql = "UPDATE `Session` SET `status`=0 WHERE `id`=?";
        auto data = std::make_tuple(id);
        return !handle->update(sql, data) ? 0 : id;
    }

    std::string sql = "INSERT INTO `Session` (`user`, `peer`, `type`, `status`) VALUES (?, ?, ?, 0)";
    auto data = std::make_tuple(from, to, type);
    if (!handle->insert(sql, data))
        return 0;

    return this->getSession(from, to, type, false);
}

uint32_t Session::getRelation(uint32_t from, uint32_t to, bool insert)
{
    auto handle = mg::MysqlConnectionPool::get().getHandle();
    if (!handle)
        return 0;

    std::string sql = "SELECT `id` FROM `RelationShip` WHERE `user`=? AND `peer`=? AND `status`=0";
    auto data = std::make_tuple(from, to);
    if (!handle->select(sql, data))
        return insert ? this->addRelation(from, to) : 0;
    return handle->next() ? handle->getData("id") : 0;
}

uint32_t Session::addRelation(uint32_t from, uint32_t to)
{
    auto handle = mg::MysqlConnectionPool::get().getHandle();
    if (!handle)
        return 0;

    {
        std::string sql = "SELECT `id` FROM `RelationShip` WHERE `user`=? AND `peer`=?";
        std::array<std::string, 1> column = {"id"};
        auto data = std::make_tuple(from, to);
        if (handle->select(sql, data) && handle->next())
        {
            uint32_t id = handle->getData("id");
            sql = "UPDATE `RelationShip` SET `status`=0, `updateTime`=? WHERE `id`=?";
            auto data = std::make_tuple(mg::TimeStamp::now().getSeconds(), id);
            return handle->update(sql, data) ? id : 0;
        }
    }

    {
        std::string sql = "INSERT INTO `RelationShip`(`user`, `peer`, `createTime`, `updateTime`) \
                           VALUES (?, ?, ?, ?)";
        mg::TimeStamp curTime = mg::TimeStamp::now();
        auto data = std::make_tuple(from, to, curTime.getSeconds(), curTime.getSeconds());
        if (!handle->insert(sql, data))
            return 0;
    }

    return this->getRelation(from, to);
}

void Session::saveMessage(uint32_t relation, IM::Message::MessageData &message)
{
    auto handle = mg::MysqlConnectionPool::get().getHandle();
    if (!handle)
        return;

    std::string sql = "INSERT INTO `MessageContent` (`relation`, `user`, `peer`, `type`, `status`, `createTime`, `content`, `messageId`) \
                       VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    auto data = std::make_tuple(relation, message.from(), message.to(), static_cast<int>(message.message_type()),
                                0, message.create_time(), std::ref(message.message_data()), message.message_id());
    handle->insert(sql, data);
}