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

#include <string>
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

        template <typename T>
        bool insert(const std::string &tablename, std::vector<std::string> &column, T &data)
        {
            std::string sql = this->parseInsert(tablename, column);
            if (!this->prepareStatement(sql))
                return false;

            this->bindParam(data);

            if (mysql_stmt_bind_param(_stmt, _bind_param) != 0)
                return false;

            return mysql_stmt_execute(_stmt) == 0;
        }

        bool insert(const std::string &sql);

        bool remove(const std::string &sql);

        bool update(const std::string &sql);

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
        std::string parseInsert(const std::string &tablename, std::vector<std::string> &column);

        bool parseUpdate(const std::string &sql);

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
            if (std::is_same<T, int>::value)
            {
                this->_bind_param[index].buffer_type = MYSQL_TYPE_LONG;
                this->_bind_param[index].buffer = &data;
            }
            else if (std::is_same<T, short>::value)
            {
                this->_bind_param[index].buffer_type = MYSQL_TYPE_SHORT;
                this->_bind_param[index].buffer = &data;
            }
            else if (std::is_same<T, bool>::value)
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
        }
    };

    // 特化处理字符串
    template <>
    void Mysql::bindHelper(std::string &data, size_t index);
    // 特化处理二进制
    template <>
    void Mysql::bindHelper(std::vector<uint8_t> &data, size_t index);
};

#endif //__MG_MYSQL_H__