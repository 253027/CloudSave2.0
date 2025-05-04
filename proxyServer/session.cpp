#include "session.h"
#include "../src/base/mysql-connection-pool.h"
#include "../src/base/time-stamp.h"

#include <sstream>

uint32_t Session::getSession(uint32_t from, uint32_t to, uint32_t type, bool tomb)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return 0;

    std::array<std::string, 1> column = {"id"};
    if (!tomb)
    {
        auto data = std::make_tuple(from, to, type);
        if (!sql->select("Session", column, "user=? AND peer=? AND type=? AND status=0", data))
            return 0;
    }
    else
    {
        auto data = std::make_tuple(from, to);
        if (!sql->select("Session", column, "user=? AND peer=?", data))
            return 0;
    }

    if (sql->next())
        return sql->getData("id");

    return 0;
}

uint32_t Session::addSession(uint32_t from, uint32_t to, uint32_t type)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return 0;

    uint32_t id = this->getSession(from, to, type, true);
    std::stringstream query;

    if (id)
    {
        std::array<std::string, 1> column = {"status"};
        auto data = std::make_tuple(0, id);
        return !sql->update("Session", column, "id=", data) ? 0 : id;
    }

    std::array<std::string, 4> column = {"user", "peer", "type", "status"};
    auto data = std::make_tuple(from, to, type, "0");
    if (!sql->insert("Session", column, data))
        return 0;

    return this->getSession(from, to, type, false);
}

uint32_t Session::getRelation(uint32_t from, uint32_t to, bool insert)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return 0;
    std::array<std::string, 1> column = {"id"};
    auto data = std::make_tuple(from, to);
    if (!sql->select("RelationShip", column, "user=? AND peer=? AND status=0", data))
        return insert ? this->addRelation(from, to) : 0;
    return sql->next() ? sql->getData("id") : 0;
}

uint32_t Session::addRelation(uint32_t from, uint32_t to)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return 0;

    {
        std::array<std::string, 1> column = {"id"};
        auto data = std::make_tuple(from, to);
        if (sql->select("RelationShip", column, "user=? AND peer=?", data) && sql->next())
        {
            uint32_t id = sql->getData("id");
            std::array<std::string, 2> set = {"status", "updateTime"};
            auto data = std::make_tuple(0, mg::TimeStamp::now().getSeconds(), id);
            return sql->update("RelationShip", set, "id=", data) ? id : 0;
        }
    }

    {
        mg::TimeStamp curTime = mg::TimeStamp::now();
        std::array<std::string, 4> columns = {"user", "peer", "createTime", "updateTime"};
        auto data = std::make_tuple(from, to, curTime.getSeconds(), curTime.getSeconds());
        if (!sql->insert("RelationShip", columns, data))
            return 0;
    }

    return this->getRelation(from, to);
}

void Session::saveMessage(uint32_t relation, IM::Message::MessageData &message)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return;

    std::array<std::string, 7> columns = {"relation", "user", "peer", "type", "status", "createTime", "content"};
    auto data = std::make_tuple(relation, message.from(), message.to(), static_cast<int>(message.message_type()),
                                0, message.create_time(), std::ref(message.message_data()));
    sql->insert("MessageContent_0", columns, data);
}