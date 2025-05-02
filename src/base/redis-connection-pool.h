#ifndef __REDIS_CONNECTION_POOL_H__
#define __REDIS_CONNECTION_POOL_H__

#include "singleton.h"
#include "redis.h"
#include "json_fwd.hpp"
#include "eventloop-thread.h"

#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace mg
{
    class RedisConnection;
    class EventLoop;
    class EventLoopThread;

    class RedisConnectionPool
    {
    public:
        RedisConnectionPool(EventLoop *loop = nullptr, const std::string &name = "");

        ~RedisConnectionPool();

        /**
         * @brief initial the redis connection pool
         * @param configPath the path of config file
         * @param config the config of redis connection pool
         * @param name the name of redis connection pool
         * @return true if initial success
         * @return false if initial failed
         */
        bool initial(const std::string &configPath, const std::string &name = "");
        bool initial(nlohmann::json &config, const std::string &name = "");

        /**
         * @brief start the redis connection pool
         * @return true if start success
         * @return false if start failed
         */
        bool start();

        /**
         * @brief stop the eventloop in _thread if _thread has benn created
         */
        void quit();

        /**
         * @brief get a redis connection
         */
        std::shared_ptr<RedisConnection> getHandle();

        /**
         * @brief enable keepalive connection like mysql ping
         */
        inline void setKeepAlive() { this->_keepalive = true; }

    private:
        bool addInitial();

        void add();

        void remove();

    private:
        EventLoop *_loop;
        std::unique_ptr<mg::EventLoopThread> _thread;
        std::deque<RedisConnection *> _queue;
        std::string _name;
        std::string _host;
        std::string _password;
        uint8_t _db;
        uint16_t _port;
        uint16_t _maxsize;
        uint16_t _minsize;
        uint16_t _totalsize;
        uint16_t _timeout;
        uint32_t _idletimeout;
        std::mutex _mutex;
        std::condition_variable _condition;
        bool _keepalive;
    };

    class RedisPoolManager : public Singleton<RedisPoolManager>
    {
    public:
        ~RedisPoolManager();

        bool initial(const std::string &configPath, EventLoop *loop = nullptr);
        bool initial(nlohmann::json &config, EventLoop *loop = nullptr);

        void quit();

        std::shared_ptr<RedisConnectionPool> getPool(const std::string &poolName);

        std::shared_ptr<RedisConnection> getHandle(const std::string &poolName);

    private:
        std::map<std::string, std::shared_ptr<RedisConnectionPool>> _pools;
        EventLoop *_loop;
        std::unique_ptr<mg::EventLoopThread> _thread;
    };
};

#endif //__REDIS_CONNECTION_POOL_H__