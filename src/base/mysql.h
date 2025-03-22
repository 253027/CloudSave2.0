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

#include <string>
#include <sstream>
#include <mysql/mysql.h>

namespace mg
{
    class Mysql
    {
        friend class MysqlConnectionPool;

    public:
        Mysql();

        ~Mysql();

        bool connect(const std::string &username, const std::string &password,
                     const std::string &databasename, const std::string &ip, uint16_t port);

        template <typename T, typename U>
        bool insert(const std::string &tablename, U &column, T &data)
        {
            static_assert(is_std_array<U>::value, "insert only support std::array");
            static_assert(std::tuple_size<U>::value == std::tuple_size<T>::value,
                          "column size must equal to data size");
            std::string sql = this->parseInsert(tablename, column);
            return this->doBindAndExec(sql, data);
        }

        template <typename T, typename U>
        bool update(const std::string &tablename, U &set, const std::string &where, T &data)
        {
            static_assert(is_std_array<U>::value, "update set field only support std::array");
            std::string sql = this->parseUpdate(tablename, set, where);
            return this->doBindAndExec(sql, data);
        }

        /**
         * @brief 查询操作
         * @param sql 要执行的sql语句
         * @return true 查询成功 false 查询失败
         * @details
         *          sql = "select id, filename from table_name;"
         *          mysql.query(sql);
         *          while(mysql.next())
         *              cout << sql.getDataByColname("id") << " " << sql.getDataByColname("filename");
         *
         */
        bool query(const std::string &sql);
        bool next();
        std::string getData(const std::string &fieldname);

        bool transaction();

        bool commit();

        bool rollback();

        void refresh();

        TimeStamp getVacantTime();

    private:
        template <typename U>
        std::string parseInsert(const std::string &tablename, U &column)
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
        std::string parseUpdate(const std::string &tablename, U &set, const std::string &where)
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

        void freeResult();

    private:
        MYSQL *_handle;
        MYSQL_RES *_res;
        MYSQL_ROW _row;
        MYSQL_FIELD *_field;
        mg::TimeStamp _alvieTime;
        MYSQL_STMT *_stmt;
        MYSQL_BIND *_bind_param;

    private:
        bool prepareStatement(const std::string &sql);

        template <typename T>
        struct is_std_array : std::false_type
        {
        };

        template <typename T, std::size_t N>
        struct is_std_array<std::array<T, N>> : std::true_type
        {
        };

        template <typename T>
        bool doBindAndExec(const std::string &sql, T &data)
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
        void bindHelper(T &data, size_t index)
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
    };

    // 特化处理字符串
    template <>
    void Mysql::bindHelper(std::string &data, size_t index);
    template <>
    void Mysql::bindHelper(const char *&data, size_t index);
    // 特化处理二进制
    template <>
    void Mysql::bindHelper(std::vector<uint8_t> &data, size_t index);
};

#endif //__MG_MYSQL_H__