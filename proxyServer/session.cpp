#include "session.h"
#include "../src/base/mysql-connection-pool.h"
#include "../src/base/time-stamp.h"

#include <sstream>

uint32_t Session::getSession(uint32_t from, uint32_t to, uint32_t type, bool tomb)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return 0;

    std::stringstream query;
    query << "SELECT `id` FROM `Session` WHERE `user`=" << from << " AND `peer`=" << to;
    if (!tomb)
        query << " AND `type`=" << type << " AND status=0";

    if (!sql->query(query.str()))
        return 0;

    if (sql->next())
        return std::stoi(sql->getData("id"));

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
    std::stringstream query;
    query << "SELECT `id` FROM `RelationShip` WHERE `user`=" << from << " AND `peer`=" << to << " AND `status`=0";
    if (!sql->query(query.str()))
        return insert ? this->addRelation(from, to) : 0;
    return sql->next() ? std::stoi(sql->getData("id")) : 0;
}

uint32_t Session::addRelation(uint32_t from, uint32_t to)
{
    auto sql = mg::MysqlConnectionPool::get().getHandle();
    if (!sql)
        return 0;

    std::stringstream query;
    query << "SELECT `id` FROM `RelationShip` WHERE `user` =" << from << " AND `peer`=" << to;
    if (sql->query(query.str()) && sql->next())
    {
        uint32_t id = std::stoi(sql->getData("id"));
        std::array<std::string, 2> set = {"status", "updateTime"};
        auto data = std::make_tuple(0, mg::TimeStamp::now().getSeconds(), id);
        return sql->update("RelationShip", set, "id=", data) ? id : 0;
    }

    mg::TimeStamp curTime = mg::TimeStamp::now();
    std::array<std::string, 4> columns = {"user", "peer", "createTime", "updateTime"};
    auto data = std::make_tuple(from, to, curTime.getSeconds(), curTime.getSeconds());
    if (!sql->insert("RelationShip", columns, data))
        return 0;

    return this->getRelation(from, to);
}