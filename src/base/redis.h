#ifndef __MG_REDIS_H__
#define __MG_REDIS_H__

#include "singleton.h"
#include "log.h"
#include "time-stamp.h"
#include "../../hiredis-1.3.0/hiredis.h"

#include <stddef.h>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <vector>
#include <list>

enum RedisReplyType
{
    REDIS_REPLY_TYPE_UNKNOWN,
    REDIS_REPLY_TYPE_STRING,
    REDIS_REPLY_TYPE_ARRAY,
    REDIS_REPLY_TYPE_INTEGER,
    REDIS_REPLY_TYPE_NIL,
    REDIS_REPLY_TYPE_STATUS,
    REDIS_REPLY_TYPE_ERROR
};

namespace mg
{
    struct RedisValue
    {
        RedisReplyType type;
        std::string str;
        long long value;
        std::vector<std::string> array;

        RedisValue() : type(REDIS_REPLY_TYPE_UNKNOWN), value(0) {}

        RedisValue(const char *str, size_t len)
            : type(REDIS_REPLY_TYPE_STRING), str(str, len) {}

        RedisValue(const std::string &str) : RedisValue(str.c_str(), str.size()) {}

        inline operator int()
        {
            if (type != REDIS_REPLY_TYPE_INTEGER)
                throw std::runtime_error("RedisValue::operator size_t()");
            return value;
        }

        inline operator uint32_t()
        {
            if (type != REDIS_REPLY_TYPE_INTEGER)
                throw std::runtime_error("RedisValue::operator size_t()");
            return value;
        }

        inline operator size_t()
        {
            if (type != REDIS_REPLY_TYPE_INTEGER)
                throw std::runtime_error("RedisValue::operator size_t()");
            return value;
        }

        inline operator std::string()
        {
            if (type != REDIS_REPLY_TYPE_STRING &&
                type != REDIS_REPLY_TYPE_NIL &&
                type != REDIS_REPLY_TYPE_ERROR &&
                type != REDIS_REPLY_TYPE_STATUS)
            {
                throw std::runtime_error("RedisValue::operator std::string()");
            }
            return str;
        }

        inline operator std::vector<std::string>()
        {
            if (type != REDIS_REPLY_TYPE_ARRAY)
                throw std::runtime_error("RedisValue::operator std::vector<std::string>()");
            return array;
        }

        inline void reset()
        {
            this->type = REDIS_REPLY_TYPE_UNKNOWN;
            this->str.clear();
            this->value = 0;
            this->array.clear();
        }
    };

    using RedisResult = RedisValue;

    class RedisConnection
    {
        friend class RedisConnectionPool;

    public:
        ~RedisConnection();

        bool connect(const std::string &ip, uint16_t port, const std::string &password,
                     int db = 0, int timeout = 0);

        bool execute(const std::string &command, RedisResult &result);

        bool selectDatabase(int db);

    public: // Here are all the commonly used Redis command methods
        bool GET(const std::string &key, RedisResult &result);

        bool SET(const std::string &key, const std::string &value);

        /**
         * @brief Retrieve values in bulk. If the operation is successful,
         *        the returned values array will correspond one-to-one with the queries
         *
         * @param keys query parameters
         * @param values if return true, values will be filled
         * @param return true if success
         * @param return false if failed
         */
        bool MGET(std::initializer_list<std::string> &&keys, RedisResult &values);
        template <typename T>
        bool MGET(T &&keys, RedisResult &values);

        /**
         * @brief Set values in bulk
         */
        bool MSET(std::initializer_list<std::string> &&keys, std::vector<std::string> &values);
        template <typename T>
        bool MSET(T &&keys, std::vector<std::string> &values);

        template <typename T = RedisResult>
        bool INCR(const std::string &key, T &&result = T());

        template <typename T = RedisResult>
        bool INCRBY(const std::string &key, int64_t value, T &&result = T());

        template <typename T = RedisResult>
        bool DECR(const std::string &key, T &&result = T());

        template <typename T = RedisResult>
        bool DECRBY(const std::string &key, int64_t value, T &&result = T());

        bool DEL(const std::string &key);

    private:
        bool authenticate(const std::string &password);

        void freeReply();

        bool checkReply();

        bool parseReply(RedisResult &result);

        bool execByParams(std::vector<std::string> &params);

        inline TimeStamp getVacantTime()
        {
            return mg::TimeStamp::now() - this->_lastExecTime;
        }

        inline void refresh()
        {
            this->_lastExecTime = mg::TimeStamp::now();
        }

    private:
        std::string _ip;
        uint16_t _port;
        std::string _password;
        int _db;
        redisContext *_context;
        redisReply *_reply;
        RedisResult _temp;
        TimeStamp _lastExecTime;
    };

    template <typename T>
    inline bool RedisConnection::MGET(T &&keys, RedisResult &values)
    {
        std::vector<std::string> _keys = {"MGET"};
        _keys.insert(_keys.end(), keys.begin(), keys.end());
        if (!this->execByParams(_keys))
            return false;
        return this->parseReply(values);
    }

    template <typename T>
    inline bool RedisConnection::MSET(T &&keys, std::vector<std::string> &values)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, std::vector<std::string>>::value,
                      "RedisConnection::MSET only support std::vector<std::string>");
        std::vector<std::string> _keys = {"MSET"};
        int len = std::min(keys.size(), values.size());
        for (int i = 0; i < len; i++)
        {
            _keys.push_back(keys[i]);
            _keys.push_back(values[i]);
        }
        return this->execByParams(_keys);
    }

    template <typename T>
    inline bool RedisConnection::INCR(const std::string &key, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::INCR() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "INCR %s", key.c_str()));
        return this->parseReply(result);
    }

    template <typename T>
    inline bool RedisConnection::INCRBY(const std::string &key, int64_t value, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::INCRBY() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context,
                                                              "INCRBY %s %lld", key.c_str(), value));
        return this->parseReply(result);
    }

    template <typename T>
    inline bool RedisConnection::DECR(const std::string &key, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::INCR() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "DECR %s", key.c_str()));
        return this->parseReply(result);
    }

    template <typename T>
    inline bool RedisConnection::DECRBY(const std::string &key, int64_t value, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::DECRBY() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context,
                                                              "DECRBY %s %lld", key.c_str(), value));
        return this->parseReply(result);
    }
};

#endif //__MG_REDIS_H__