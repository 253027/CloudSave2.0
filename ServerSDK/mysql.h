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
    /**
     * 定义结构体：
     *              struct name
     *              {
     *                  类型1 变量名1
     *                  类型2 变量名2
     *              }__attribute__((packed));
     *
     * 定义结构体描述和插入键值名：
     *              DataField name[] =
     *              {
     *                {"数据列名1", "结构体成员类型", "成员大小所占字节数"},
     *                 .......,
     *                {nullptr, DB_INVALID, 0},      //最后一列必须以nullptr结束
     *              };
     *
     * example:
     *          struct test
     *          {
     *              char buf[33];
     *              int a;
     *          }__attribute__((packed));
     *
     *          mg::DataField field[] =
     *          {
     *              {"name",   mg::DataType::DB_STRING, 33},
     *              {"score",  mg::DataType::DB_INT32, sizeof(int)},
     *              { nullptr, mg::DataType::DB_INVALID, 0},
     *          };
     *
     *          std::underlying_type_t<DataType> color_value = static_cast<std::underlying_type_t<DataType>>(Color::DB_STRING)
     *
     */
    enum class DataType
    {
        DB_STRING = 1, // C风格字符串数据
        DB_CHAR,       // 单字节数据
        DB_INT32,      // 32位整型数据
        DB_BIN,        // 二进制数据
        DB_INVALID     // 无效位
    };

    struct DataField
    {
        char *name;    // 数据名
        DataType type; // 数据类型
        int size;      // 数据大小(上限)
    };

    class Mysql
    {
    public:
        Mysql();

        ~Mysql();

        /**
         * @brief 连接至mysql数据库
         * @param username 用户名
         * @param password 密码
         * @param databasename 数据名
         * @param port 端口
         * @return true 连接成功 false 连接失败
         */
        bool connect(const std::string &username, const std::string &password,
                     const std::string &databasename, const std::string &ip, uint16_t port);

        /**
         * @brief 插入操作
         * @param tablename 表名
         * @param column 插入表类型
         * @param data 插入数据
         * @return true 插入成功 false 插入失败
         */
        bool insert(const std::string &tablename, DataField *column, char *data);
        /**
         * @param sql 要执行的sql语句
         */
        bool insert(const std::string &sql);

        /**
         * @brief 删除操作
         * @param sql 要执行的sql语句
         * @return true 删除成功 false 删除失败
         */
        bool remove(const std::string &sql);

        /**
         * @brief 更新操作
         * @param sql 要执行的sql语句
         * @return true 删除成功 false 删除失败
         */
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

        /**
         * @brief 事务
         */
        bool transaction();
        /**
         * @brief 提交事务
         */
        bool commit();

        /**
         * @brief huigun
         */
        bool rollback();

        /**
         * @brief 刷新空闲起始时间
         */
        void refresh();

        /**
         * @brief 获取空闲时间
         */
        TimeStamp getVacantTime();

    private:
        std::string parseInsert(const std::string &tablename, DataField *column, char *data);

        bool parseUpdate(const std::string &sql);

        void freeResult();

        MYSQL *_handle;
        MYSQL_RES *_res;
        MYSQL_ROW _row;
        MYSQL_FIELD *_field;
        mg::TimeStamp _alvieTime;
    };
};

#endif //__MG_MYSQL_H__