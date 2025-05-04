#include "mysql.h"
#include <string.h>

mg::Mysql::Mysql() : _handle(mysql_init(nullptr)), _res(nullptr),
                     _row(nullptr), _field(nullptr), _alvieTime(0),
                     _stmt(nullptr), _bind_param(nullptr), _store_bind(nullptr)
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

mg::AnyType mg::Mysql::getData(const std::string &fieldname)
{
    if (!_res || !_field)
    {
        LOG_ERROR("Result set or field is null");
        return AnyType();
    }

    int index = -1, colNums = mysql_num_fields(_res);
    for (int i = 0; i < colNums; i++)
    {
        if (!::strcmp(fieldname.c_str(), _field[i].name))
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        LOG_ERROR("Unknown Column: {}", fieldname);
        return AnyType();
    }

    MYSQL_BIND *bind = this->_store_bind + index;
    if (!bind->buffer)
    {
        LOG_ERROR("Buffer for column {} is null", fieldname);
        return AnyType();
    }

    return AnyType(static_cast<char *>(bind->buffer), *(bind->length));
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
    if (!_stmt)
        return false;
    int ret = mysql_stmt_fetch(_stmt);
    if (ret && ret != MYSQL_DATA_TRUNCATED)
    {
        this->freeResult();
        return false;
    }
    int numsFields = mysql_num_fields(_res);
    for (int i = 0; i < numsFields; i++)
    {
        MYSQL_BIND *bind = &this->_store_bind[i];
        uint32_t real_length = *bind->length;
        if (real_length < 0)
            return false;
        if (bind->buffer == nullptr || (bind->buffer_length < real_length))
        {
            char *free_buffer = static_cast<char *>(bind->buffer);
            SAFE_DELETE_ARRAY(free_buffer);
            bind->buffer = new char[real_length]();
            bind->buffer_length = real_length;
            if (mysql_stmt_fetch_column(this->_stmt, bind, i, 0))
            {
                LOG_ERROR("mysql_stmt_fetch_column: {}", mysql_stmt_error(_stmt));
                return false;
            }
        }
    }
    return true;
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
    {
        mysql_stmt_close(_stmt);
        _stmt = nullptr;
    }
    int numsFields = 0;
    if (_res)
    {
        numsFields = mysql_num_fields(_res);
        mysql_free_result(_res);
        _res = nullptr;
    }
    if (_store_bind)
    {
        for (int i = 0; i < numsFields; i++)
        {
            MYSQL_BIND *bind = &this->_store_bind[i];
            SAFE_DELETE(bind->is_null);
            SAFE_DELETE(bind->length);
            char *buffer = static_cast<char *>(bind->buffer);
            SAFE_DELETE_ARRAY(buffer);
        }
        SAFE_DELETE_ARRAY(_store_bind);
    }
    _field = nullptr;
    _row = nullptr;
    SAFE_DELETE_ARRAY(_bind_param);
}

bool mg::Mysql::prepareStatement(const std::string &sql)
{
    this->freeResult();
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

bool mg::Mysql::storeResult()
{
    if (this->_res)
        mysql_free_result(this->_res);
    this->_res = mysql_stmt_result_metadata(_stmt);
    if (!_res)
    {
        LOG_ERROR("mysql_stmt_result_metadata: {}", mysql_stmt_error(_stmt));
        return false;
    }
    int numsFields = mysql_num_fields(_res);
    this->_field = mysql_fetch_fields(this->_res);
    this->_store_bind = new MYSQL_BIND[numsFields]();
    for (int i = 0; i < numsFields; i++)
    {
        MYSQL_BIND *bind = &this->_store_bind[i];
        bind->is_null = new bool();
        bind->length = new unsigned long();
        bind->buffer_type = this->_field[i].type;
        switch (this->_field[i].type)
        {
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        {
            bind->buffer = nullptr;
            bind->buffer_length = 0;
            break;
        }
        default:
        {
            bind->buffer = new char[64]();
            bind->buffer_length = 64;
            break;
        }
        }
    }
    if (mysql_stmt_bind_result(_stmt, this->_store_bind))
    {
        LOG_ERROR("mysql_stmt_bind_result: {}", mysql_stmt_error(_stmt));
        return false;
    }
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