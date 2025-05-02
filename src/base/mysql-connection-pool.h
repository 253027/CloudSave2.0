#ifndef __MG_MYSQL_CONNECTION_POOL_H__
#define __MG_MYSQL_CONNECTION_POOL_H__

#include "json_fwd.hpp"
#include "singleton.h"
#include "thread.h"
#include "timer-queue.h"
#include "mysql.h"

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <string>

namespace mg
{
    class Mysql;
    class EventLoop;
    class EventLoopThread;
    class MysqlConnectionPool : public Singleton<MysqlConnectionPool>
    {
    public:
        MysqlConnectionPool();

        ~MysqlConnectionPool();

        /**
         * @brief 初始化
         * @param configPath 配置文件路径
         * @param name 连接池名
         * @return true 初始化成功 false 初始化失败
         */
        bool initial(const std::string &configPath, const std::string &name = "");
        bool initial(nlohmann::json &config, const std::string &name = "");

        /**
         * @param keeplive 是否定时发送心跳包（单位秒）
         */
        bool start(int keeplive = 0);

        /**
         * @brief 停止线程池
         */
        void quit();

        /**
         * @brief 获取一个数据库实例
         */
        std::shared_ptr<Mysql> getHandle();

    private:
        /**
         * @brief 连接回收函数
         */
        void remove();

        /**
         * @brief 连接增加函数
         */
        void add();

        /**
         * @brief 初始化连接
         * @return true 初始化成功 false 初始化失败
         */
        bool addInitial();

        /**
         * @brief 定时发送心跳包
         */
        void keepAlive();

        std::string _host;
        std::string _username;
        std::string _password;
        std::string _databasename;
        uint16_t _port;
        uint16_t _maxsize;
        uint16_t _minsize;
        uint16_t _totalsize;
        uint16_t _timeout;
        uint32_t _idletimeout;
        std::queue<Mysql *> _queue;
        std::mutex _mutex;
        std::condition_variable _condition;
        EventLoop *_loop;
        std::unique_ptr<mg::EventLoopThread> _thread;
    };
};

#endif //__MG_MYSQL_CONNECTION_POOL_H__