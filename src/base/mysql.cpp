#include "mysql.h"
#include <string.h>
#include "macros.h"

mg::Mysql::Mysql() : _handle(mysql_init(nullptr)), _res(nullptr),
                     _row(nullptr), _field(nullptr), _alvieTime(0),
                     _stmt(nullptr), _bind_param(nullptr)
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

bool mg::Mysql::query(const std::string &sql)
{
    freeResult();
    if (mysql_real_query(_handle, sql.c_str(), sql.size()))
    {
        LOG_ERROR("\nQuery: {}\nErrno: {}\nErrors: {}", sql, mysql_errno(_handle), mysql_error(_handle));
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

void mg::Mysql::freeResult()
{
    if (_stmt)
        mysql_stmt_close(_stmt);
    _stmt = nullptr;
    SAFE_DELETE_ARRAY(_bind_param);
    if (_res == nullptr)
        return;
    mysql_free_result(_res);
    _res = nullptr;
    _field = nullptr;
    _row = nullptr;
}

bool mg::Mysql::prepareStatement(const std::string &sql)
{
    SAFE_DELETE_ARRAY(this->_bind_param);
    if (this->_stmt)
        mysql_stmt_close(this->_stmt);

    this->_stmt = mysql_stmt_init(this->_handle);
    if (!this->_stmt)
    {
        LOG_ERROR("mysql_stmt_init: {}", mysql_error(this->_handle));
        return false;
    }

    if (mysql_stmt_prepare(this->_stmt, sql.c_str(), sql.size()))
    {
        LOG_ERROR("mysql_stmt_prepare: {}", mysql_stmt_error(this->_stmt));
        return false;
    }

    int num = mysql_stmt_param_count(this->_stmt);
    if (num > 0)
        this->_bind_param = new MYSQL_BIND[num]();

    return true;
}

template <>
void mg::Mysql::bindHelper(std::string &data, size_t index)
{
    this->_bind_param[index].buffer_type = MYSQL_TYPE_STRING;
    this->_bind_param[index].buffer = const_cast<char *>(data.c_str());
    this->_bind_param[index].buffer_length = data.size();
}

template <>
void mg::Mysql::bindHelper(const std::string &data, size_t index)
{
    this->_bind_param[index].buffer_type = MYSQL_TYPE_STRING;
    this->_bind_param[index].buffer = const_cast<char *>(data.c_str());
    this->_bind_param[index].buffer_length = data.size();
}

template <>
void mg::Mysql::bindHelper(const char *&data, size_t index)
{
    this->_bind_param[index].buffer_type = MYSQL_TYPE_STRING;
    this->_bind_param[index].buffer = const_cast<char *>(data);
    this->_bind_param[index].buffer_length = ::strlen(data);
}

template <>
void mg::Mysql::bindHelper(std::vector<uint8_t> &data, size_t index)
{
    this->_bind_param[index].buffer_type = MYSQL_TYPE_BLOB;
    this->_bind_param[index].buffer = data.data();
    this->_bind_param[index].buffer_length = data.size();
    this->_bind_param[index].length = &this->_bind_param[index].buffer_length;
}