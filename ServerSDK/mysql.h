/**
 * @brief mysql官方C语言API二次封装
 *
 * @author mogaitesheng
 *
 * @date 2024-07-07
 */

#ifndef __MG_MYSQL_H__
#define __MG_MYSQL_H__

#include <string>
#include "singleton.h"
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
     *             char buf[33];
     *              int a;
     *          }__attribute__((packed));
     *
     *          mg::mysql::DataField field[] =
     *          {
     *              {"name",   mg::mysql::DATATYPE::DB_STRING, 33},
     *              {"score",  mg::mysql::DATATYPE::DB_INT32, sizeof(int)},
     *              { nullptr, mg::mysql::DATATYPE::DB_INVALID, 0},
     *          };
     */
    namespace mysql
    {
        enum DATATYPE
        {
            DB_STRING = 1, // C风格字符串数据
            DB_CHAR,       // 单字节数据
            DB_INT32,      // 32位整型数据
            DB_BIN,        // 二进制数据
            DB_INVALID     // 无效位
        };

        enum CALCTYPE
        {
            DB_AUTO = 0,
        };

        struct DataField
        {
            char *name;    // 数据名
            DATATYPE type; // 数据类型
            int size;      // 数据大小(上限)
        };
    };

    class Mysql : public Singleton<Mysql>
    {
    public:
        Mysql();

        ~Mysql();

    public:
        /**
         * @brief 将要存入的数据结构转换成插入语句
         * @param name 插入的表名
         * @param column 数据结构描述列
         * @return 生成sql语句
         */
        std::string parseInsert(const std::string &name, mysql::DataField *column, char *data);

        MYSQL *handle;
    };
};

#endif //__MG_MYSQL_H__