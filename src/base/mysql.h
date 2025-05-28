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
#include <type_traits>
#include <sstream>
#include <mysql/mysql.h>

namespace mg
{
    class AnyType
    {
    public:
        AnyType() {}

        AnyType(char *buffer, size_t size)
        {
            this->_data.assign(buffer, buffer + size);
        }

        template <typename T,
                  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        inline operator T()
        {
            if (std::is_integral<T>::value)
            {
                if (std::is_unsigned<T>::value)
                {
                    switch (this->_data.size())
                    {
                    case sizeof(uint8_t):
                        return *(reinterpret_cast<const uint8_t *>(this->_data.data()));
                    case sizeof(uint16_t):
                        return *(reinterpret_cast<const uint16_t *>(this->_data.data()));
                    case sizeof(uint32_t):
                        return *(reinterpret_cast<const uint32_t *>(this->_data.data()));
                    case sizeof(uint64_t):
                        return *(reinterpret_cast<const uint64_t *>(this->_data.data()));
                    default:
                        throw std::runtime_error("Unsupported size for unsigned integral type");
                    }
                }
                else
                {
                    switch (this->_data.size())
                    {
                    case sizeof(int8_t):
                        return *(reinterpret_cast<const int8_t *>(this->_data.data()));
                    case sizeof(int16_t):
                        return *(reinterpret_cast<const int16_t *>(this->_data.data()));
                    case sizeof(int32_t):
                        return *(reinterpret_cast<const int32_t *>(this->_data.data()));
                    case sizeof(int64_t):
                        return *(reinterpret_cast<const int64_t *>(this->_data.data()));
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
                    return *(reinterpret_cast<const float *>(this->_data.data()));
                case sizeof(double):
                    return *(reinterpret_cast<const double *>(this->_data.data()));
                default:
                    throw std::runtime_error("Unsupported size for floating point type");
                }
            }

            throw std::runtime_error("Unsupported type conversion");
        }

        inline operator std::string()
        {
            return std::move(this->_data);
        }

        template <typename T,
                  typename = typename std::enable_if<std::is_same<T, uint8_t>::value ||
                                                     std::is_same<T, int8_t>::value ||
                                                     std::is_same<T, char>::value ||
                                                     std::is_same<T, u_char>::value>::type>
        inline operator std::vector<T>()
        {
            return {this->_data.begin(), this->_data.end()};
        }

        template <typename U>
        inline bool operator==(const U &data)
        {
            return data == this->operator U();
        }

        template <typename U>
        inline bool operator!=(const U &data)
        {
            return data != this->operator U();
        }

        template <typename U>
        inline bool operator<(const U &data)
        {
            return data < this->operator U();
        }

        template <typename U>
        inline bool operator>(const U &data)
        {
            return data > this->operator U();
        }

        template <typename U>
        inline bool operator<=(const U &data)
        {
            return data <= this->operator U();
        }

        template <typename U>
        inline bool operator>=(const U &data)
        {
            return data >= this->operator U();
        }

        inline bool operator==(const std::string &data)
        {
            return data == this->_data;
        }

        inline bool operator!=(const std::string &data)
        {
            return data != this->_data;
        }

    private:
        std::string _data;
    };

    class Mysql
    {
        friend class MysqlConnectionPool;

        template <typename T>
        struct is_std_tuple : std::false_type
        {
        };

        template <typename... Args>
        struct is_std_tuple<std::tuple<Args...>> : std::true_type
        {
        };

        template <typename T>
        using decay_t = typename std::decay<T>::type;

    public:
        Mysql();

        ~Mysql();

        bool connect(const std::string &username, const std::string &password,
                     const std::string &databasename, const std::string &ip, uint16_t port);

        AnyType getData(const std::string &fieldname);

        template <typename T = std::tuple<>,
                  typename = typename std::enable_if<is_std_tuple<decay_t<T>>::value>::type>
        bool select(const std::string &sql, T &&condition = T())
        {
            if (!this->doBindAndExec(sql, condition))
                return false;
            return this->storeResult();
        }

        template <typename T,
                  typename = typename std::enable_if<is_std_tuple<decay_t<T>>::value>::type>
        bool insert(const std::string &sql, T &&data = T())
        {
            return this->doBindAndExec(sql, data);
        }

        template <typename T,
                  typename = typename std::enable_if<is_std_tuple<decay_t<T>>::value>::type>
        bool update(const std::string &sql, T &&data = T())
        {
            return this->doBindAndExec(sql, data);
        }

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