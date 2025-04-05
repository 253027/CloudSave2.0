/**
 * @brief mysql官方C语言API二次封装
 *
 * @author mogaitesheng
 *
 * @date 2024-07-07
 */

#ifndef __MG_MYSQL_H__
#define __MG_MYSQL_H__

#include "time-stamp.h"
#include "log.h"
#include "macros.h"

#include <string>
#include <sstream>
#include <mysql/mysql.h>

namespace mg
{
    template <typename T>
    class AnyType
    {
    public:
        AnyType() {}

        AnyType(char *buffer, size_t size)
        {
            this->_data.assign(buffer, buffer + size);
        }

        operator T()
        {
            if (std::is_integral<T>::value)
            {
                if (std::is_unsigned<T>::value)
                {
                    switch (this->_data.size())
                    {
                    case 1:
                        return *(reinterpret_cast<uint8_t *>(this->_data.data()));
                    case 2:
                        return *(reinterpret_cast<uint16_t *>(this->_data.data()));
                    case 4:
                        return *(reinterpret_cast<uint32_t *>(this->_data.data()));
                    case 8:
                        return *(reinterpret_cast<uint64_t *>(this->_data.data()));
                    default:
                        throw std::runtime_error("Unsupported size for unsigned integral type");
                    }
                }
                else
                {
                    switch (this->_data.size())
                    {
                    case 1:
                        return *(reinterpret_cast<int8_t *>(this->_data.data()));
                    case 2:
                        return *(reinterpret_cast<int16_t *>(this->_data.data()));
                    case 4:
                        return *(reinterpret_cast<int32_t *>(this->_data.data()));
                    case 8:
                        return *(reinterpret_cast<int64_t *>(this->_data.data()));
                    default:
                        throw std::runtime_error("Unsupported size for signed integral type");
                    }
                }
            }
            else if (std::is_floating_point<T>::value)
            {
                switch (this->_data.size())
                {
                case sizeof(float):
                    return *(reinterpret_cast<float *>(this->_data.data()));
                case sizeof(double):
                    return *(reinterpret_cast<double *>(this->_data.data()));
                default:
                    throw std::runtime_error("Unsupported size for floating point type");
                }
            }

            throw std::runtime_error("Unsupported type conversion");
        }

        template <typename U>
        bool operator==(const U &data)
        {
            return data == this->operator U();
        }

        template <typename U>
        bool operator!=(const U &data)
        {
            return data != this->operator U();
        }

    private:
        std::vector<u_char> _data;
    };

    template <>
    class AnyType<std::string>
    {
    public:
        AnyType() {}

        AnyType(char *buffer, size_t size) : _data(buffer, buffer + size) {}

        operator std::string()
        {
            return this->_data;
        }

        bool operator==(const std::string &data)
        {
            return data == this->_data;
        }

        bool operator!=(const std::string &data)
        {
            return data != this->_data;
        }

    private:
        std::string _data;
    };

    class Mysql
    {
        friend class MysqlConnectionPool;

    public:
        Mysql();

        ~Mysql();

        bool connect(const std::string &username, const std::string &password,
                     const std::string &databasename, const std::string &ip, uint16_t port);

        template <typename T>
        AnyType<T> getData(const std::string &fieldname);

        template <typename U, typename T = std::tuple<>>
        bool select(const std::string &tablename, U &column, const std::string &where = "", T &data = std::tuple<>());

        template <typename T, typename U>
        bool insert(const std::string &tablename, U &column, T &data);

        template <typename T, typename U>
        bool update(const std::string &tablename, U &set, const std::string &where, T &data);

        bool query(const std::string &sql);

        bool next();

        bool transaction();

        bool commit();

        bool rollback();

        void refresh();

        TimeStamp getVacantTime();

    private:
        bool prepareStatement(const std::string &sql);

        bool storeResult();

        void freeResult();

    private:
        template <typename U>
        std::string parseInsert(const std::string &tablename, U &column);

        template <typename U>
        std::string parseUpdate(const std::string &tablename, U &set, const std::string &where);

        template <typename U>
        std::string parseSelect(const std::string &tablename, U &set, const std::string &where);

        template <typename T>
        struct is_std_array : std::false_type
        {
        };

        template <typename T, std::size_t N>
        struct is_std_array<std::array<T, N>> : std::true_type
        {
        };

        template <typename T>
        bool doBindAndExec(const std::string &sql, T &data);

        template <typename Tuple, size_t Index = 0>
        typename std::enable_if<Index == std::tuple_size<Tuple>::value>::type
        bindParam(Tuple &t) {}

        template <typename Tuple, size_t Index = 0>
            typename std::enable_if < Index<std::tuple_size<Tuple>::value>::type bindParam(Tuple &t)
        {
            this->bindHelper(std::get<Index>(t), Index);
            this->bindParam<Tuple, Index + 1>(t);
        }

        template <typename T>
        void bindHelper(T &data, size_t index);

    private:
        MYSQL *_handle;
        MYSQL_RES *_res;
        MYSQL_ROW _row;
        MYSQL_FIELD *_field;
        mg::TimeStamp _alvieTime;
        MYSQL_STMT *_stmt;
        MYSQL_BIND *_bind_param;
        MYSQL_BIND *_store_bind;
    };

    template <typename T = std::string>
    AnyType<T> Mysql::getData(const std::string &fieldname)
    {
        if (!_res || !_field)
        {
            LOG_ERROR("Result set or field is null");
            return AnyType<T>();
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
            return AnyType<T>();
        }

        MYSQL_BIND *bind = this->_store_bind + index;
        unsigned long length = *(bind->length);

        if (!bind->buffer)
        {
            LOG_ERROR("Buffer for column {} is null", fieldname);
            return AnyType<T>();
        }

        return AnyType<T>(static_cast<char *>(bind->buffer), length);
    }

    template <typename U>
    std::string Mysql::parseInsert(const std::string &tablename, U &column)
    {
        std::ostringstream sql;
        sql << "INSERT INTO `" << tablename << "` (";
        std::string field, value;
        for (auto &x : column)
            field += x, field += ", ", value += "?, ";
        field = field.substr(0, field.size() - 2);
        value = value.substr(0, value.size() - 2);
        sql << field << ") VALUES (" << value << ")";
        return sql.str();
    }

    template <typename U>
    std::string Mysql::parseUpdate(const std::string &tablename, U &set, const std::string &where)
    {
        std::ostringstream sql;
        sql << "UPDATE `" << tablename << "` SET ";
        std::string field1, filed2;
        for (auto &value : set)
            field1 += value, field1 += "=?, ";
        sql << field1.substr(0, field1.size() - 2);
        if (where.empty())
            return sql.str();
        sql << " WHERE " << where;
        return sql.str();
    }

    template <typename U>
    std::string Mysql::parseSelect(const std::string &tablename, U &set, const std::string &where)
    {
        std::ostringstream sql;
        if (set.empty())
            sql << "SELECT *";
        else
        {
            sql << "SELECT `";
            for (int i = 0; i < set.size() - 1; i++)
                sql << set[i] << "`, `";
            sql << set.back() << "`";
        }
        sql << " FROM `" << tablename << "`";
        if (!where.empty())
            sql << " WHERE " << where;
        return sql.str();
    }

    template <typename U, typename T>
    bool Mysql::select(const std::string &tablename, U &column, const std::string &where, T &data)
    {
        static_assert(is_std_array<U>::value, "select only support std::array");
        std::string sql = this->parseSelect(tablename, column, where);
        if (!this->doBindAndExec(sql, data))
            return false;
        return this->storeResult();
    }

    template <typename T, typename U>
    bool Mysql::insert(const std::string &tablename, U &column, T &data)
    {
        static_assert(is_std_array<U>::value, "insert only support std::array");
        static_assert(std::tuple_size<U>::value == std::tuple_size<T>::value,
                      "column size must equal to data size");
        std::string sql = this->parseInsert(tablename, column);
        return this->doBindAndExec(sql, data);
    }

    template <typename T, typename U>
    bool Mysql::update(const std::string &tablename, U &set, const std::string &where, T &data)
    {
        static_assert(is_std_array<U>::value, "update set field only support std::array");
        std::string sql = this->parseUpdate(tablename, set, where);
        return this->doBindAndExec(sql, data);
    }

    template <typename T>
    bool Mysql::doBindAndExec(const std::string &sql, T &data)
    {
        if (!this->prepareStatement(sql))
            return false;
        this->bindParam(data);
        if (mysql_stmt_bind_param(_stmt, _bind_param) != 0)
            return false;
        if (mysql_stmt_execute(_stmt))
        {
            LOG_ERROR("\nQuery: {}\nErrno: {}\nErrors: {}", sql,
                      mysql_stmt_errno(_stmt), mysql_stmt_error(_stmt));
            mysql_stmt_free_result(_stmt);
            return false;
        }
        return true;
    }

    template <typename T>
    void Mysql::bindHelper(T &data, size_t index)
    {
        if (std::is_same<T, bool>::value)
        {
            this->_bind_param[index].buffer_type = MYSQL_TYPE_TINY;
            this->_bind_param[index].buffer = &data;
        }
        else if (std::is_same<T, float>::value)
        {
            _bind_param[index].buffer_type = MYSQL_TYPE_FLOAT;
            _bind_param[index].buffer = &data;
        }
        else if (std::is_same<T, double>::value)
        {
            _bind_param[index].buffer_type = MYSQL_TYPE_DOUBLE;
            _bind_param[index].buffer = &data;
        }
        else if (std::is_integral<T>::value)
        {
            this->_bind_param[index].buffer_type = MYSQL_TYPE_LONG;
            this->_bind_param[index].buffer = &data;
        }
    }

    // 特化处理字符串
    template <>
    void Mysql::bindHelper(std::string &data, size_t index);
    template <>
    void Mysql::bindHelper(const char *&data, size_t index);
    template <>
    void Mysql::bindHelper(const std::string &data, size_t index);
    // 特化处理二进制
    template <>
    void Mysql::bindHelper(std::vector<uint8_t> &data, size_t index);
};

#endif //__MG_MYSQL_H__