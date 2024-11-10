#include "mysql.h"
#include <sstream>
#include <string.h>
#include "macros.h"
#include "log.h"

mg::Mysql::Mysql() : _handle(mysql_init(nullptr)), _res(nullptr),
                     _row(nullptr), _field(nullptr), _alvieTime(0)
{
    mysql_set_character_set(_handle, "utf8");
    if (_handle == nullptr)
        LOG_ERROR("mysql nullptr");
}

mg::Mysql::~Mysql()
{
    freeResult();
    if (_handle)
        mysql_close(_handle);
    _handle = nullptr;
}

bool mg::Mysql::connect(const std::string &username, const std::string &password, const std::string &databasename, const std::string &ip, uint16_t port)
{
    MYSQL *res = mysql_real_connect(_handle, ip.c_str(), username.c_str(), password.c_str(), databasename.c_str(), port, nullptr, 0);
    if (res == nullptr)
        LOG_ERROR("{}", mysql_error(_handle));
    return res != nullptr;
}

bool mg::Mysql::insert(const std::string &tablename, DataField *column, char *data)
{
    return parseUpdate(parseInsert(tablename, column, data));
}

bool mg::Mysql::insert(const std::string &sql)
{
    return parseUpdate(sql);
}

bool mg::Mysql::remove(const std::string &sql)
{
    return parseUpdate(sql);
}

bool mg::Mysql::update(const std::string &sql)
{
    return parseUpdate(sql);
}

bool mg::Mysql::query(const std::string &sql)
{
    freeResult();
    if (mysql_real_query(_handle, sql.c_str(), sql.size()))
    {
        LOG_ERROR("\nQuery: {}\nErrno:{}\nErrors: {}", sql, mysql_errno(_handle), mysql_error(_handle));
        return false;
    }
    _res = mysql_store_result(_handle);
    _field = mysql_fetch_fields(_res);
    return true;
}

bool mg::Mysql::next()
{
    if (_res == nullptr)
        return false;
    _row = mysql_fetch_row(_res);
    return _row != nullptr;
}

std::string mg::Mysql::getData(const std::string &fieldname)
{
    int colNums = mysql_num_fields(_res);
    for (int i = 0; i < colNums; i++)
    {
        if (::strcmp(fieldname.c_str(), _field[i].name))
            continue;
        return (_row[i] == nullptr) ? "" : _row[i];
    }
    return "";
}

bool mg::Mysql::transaction()
{
    return mysql_autocommit(_handle, false);
}

bool mg::Mysql::commit()
{
    return mysql_commit(_handle);
}

bool mg::Mysql::rollback()
{
    return mysql_rollback(_handle);
}

void mg::Mysql::refresh()
{
    _alvieTime = mg::TimeStamp::now();
}

mg::TimeStamp mg::Mysql::getVacantTime()
{
    return mg::TimeStamp::now() - _alvieTime;
}

std::string mg::Mysql::parseInsert(const std::string &name, DataField *column, char *data)
{
    int offset = 0;
    std::ostringstream sql;
    sql << "insert into `" << name << "` (";
    std::string field, value;
    for (DataField *p = column; p->name; p++)
    {
        if (!::strlen(p->name))
            continue;
        field += p->name, field += ", ";
        int len = 0;
        switch (p->type)
        {
        case DataType::DB_STRING:
        {
            len = ::strlen(data + offset);
            if (p->size)
                len = std::min(len, p->size);
            std::string buf(len * 2 + 3, '\'');
            len = mysql_real_escape_string(_handle, buf.data() + 1, data + offset, len);
            buf.resize(len + 2);
            buf.back() = '\'';
            value += buf;
            break;
        }
        case DataType::DB_INT32:
        {
            value += std::to_string(*((int *)(data + offset)));
            break;
        }
        }
        value += ", ";
        offset += p->size ? p->size : len;
    }
    field = field.substr(0, field.size() - 2);
    value = value.substr(0, value.size() - 2);
    sql << field << ") values (" << value << ")";
    return sql.str();
}

bool mg::Mysql::parseUpdate(const std::string &sql)
{
    return mysql_real_query(_handle, sql.c_str(), sql.size()) == 0;
}

void mg::Mysql::freeResult()
{
    if (_res == nullptr)
        return;
    mysql_free_result(_res);
    _res = nullptr;
    _field = nullptr;
    _row = nullptr;
}
