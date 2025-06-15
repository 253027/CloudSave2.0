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

namespace mg
{
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

    /**
     * @note When performing type conversion with `RedisResult`, converting to `std::string` or `std::vector<std::string>` will trigger `std::move`.
     *        After the move assignment is performed on the target object, `RedisResult` will no longer hold the query result.
     */
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

        template <typename T, typename = typename std::enable_if<std::is_integral<T>::value ||
                                                                 std::is_floating_point<T>::value ||
                                                                 std::is_enum<T>::value>::type>
        inline operator T()
        {
            // static_assert(std::is_integral<T>::value,
            //               "RedisValue::operator onli support interger");
            if (type != REDIS_REPLY_TYPE_INTEGER)
                throw std::runtime_error("RedisValue::operator is not interger)");
            return static_cast<T>(value);
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
            return std::move(str);
        }

        inline operator std::vector<std::string>()
        {
            if (type != REDIS_REPLY_TYPE_ARRAY)
                throw std::runtime_error("RedisValue::operator std::vector<std::string>()");
            return std::move(array);
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
        /**
         * @brief The following functions provide interfaces to access Redis.
         *  The types of execution results are the same as those returned by querying with redis-cli.
         *      For results that return an integer, you can use built-in integer type conversion.
         * Example: uint32_t value = result;
         *
         *      For results that return a list, you can store the result in a std::vector<std::string>.
         * Example: std::vector<std::string> values = result;
         *
         *      For results that return an error or a string (either due to an error or the nature of the query),
         * you can assign the result to a std::string.
         * Example: std::string value = result;
         */

        bool GET(const std::string &key, RedisResult &result);

        bool SET(const std::string &key, const std::string &value);

        bool MGET(std::initializer_list<std::string> &&keys, RedisResult &result);
        template <typename T>
        bool MGET(T &&keys, RedisResult &values);

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

        bool HGET(const std::string &key, const std::string &field, RedisResult &result);

        template <typename T = RedisResult>
        bool HSET(const std::string &key, const std::string &field, const std::string &value, T &&result = T());

        bool HGETALL(const std::string &key, RedisResult &result);

        bool HEXISTS(const std::string &key, const std::string &field, RedisResult &result);

        bool HMGET(const std::string &key, std::initializer_list<std::string> &&fields, RedisResult &result);
        template <typename T>
        bool HMGET(const std::string &key, T &fields, RedisResult &result);

        template <typename T>
        bool HMSET(const std::string &key, std::initializer_list<std::string> &&fields, T &values);
        template <typename T>
        bool HMSET(const std::string &key, T &fields, std::vector<std::string> &values);

        bool HDEL(const std::string &key, const std::string &field);
        bool HDEL(const std::string &key, std::vector<std::string> &fields);

        template <typename T = RedisResult>
        bool HINCRBY(const std::string &key, const std::string &field, int64_t value, T &&result = T());

    private:
        bool authenticate(const std::string &password);

        void freeReply();

        bool checkReply();

        template <typename T>
        bool parseReply(T &&result);

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
    inline bool RedisConnection::parseReply(T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::parseReply() only support RedisResult");
        if (!this->checkReply())
            return false;

        // if result is rvalue reference we do nothing
        if (std::is_rvalue_reference<decltype(result)>::value)
            return true;

        result.reset();
        result.type = static_cast<RedisReplyType>(this->_reply->type);
        switch (this->_reply->type)
        {
        case REDIS_REPLY_TYPE_STRING:
        case REDIS_REPLY_TYPE_STATUS:
        {
            result.str = std::string(this->_reply->str, this->_reply->len);
            break;
        }
        case REDIS_REPLY_TYPE_INTEGER:
        {
            result.value = this->_reply->integer;
            break;
        }
        case REDIS_REPLY_TYPE_ERROR:
        {
            result.str = std::string(this->_reply->str, this->_reply->len);
            return false;
        }
        case REDIS_REPLY_TYPE_ARRAY:
        {
            int count = this->_reply->elements;
            for (int i = 0; i < count; i++)
            {
                auto &reply = this->_reply->element[i];
                if (reply->str)
                    result.array.emplace_back(reply->str, reply->len);
                else
                {
                    LOG_WARN("value is null: {}", i);
                    result.array.emplace_back("");
                }
            }
            break;
        }
        case REDIS_REPLY_TYPE_NIL:
        {
            result.str = "REDIS_REPLY_TYPE_NIL";
            break;
        }
        default:
            return false;
        }
        return true;
    }

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
        return this->parseReply(std::forward<T>(result));
    }

    template <typename T>
    inline bool RedisConnection::INCRBY(const std::string &key, int64_t value, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::INCRBY() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context,
                                                              "INCRBY %s %lld", key.c_str(), value));
        return this->parseReply(std::forward<T>(result));
    }

    template <typename T>
    inline bool RedisConnection::DECR(const std::string &key, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::DECR() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "DECR %s", key.c_str()));
        return this->parseReply(std::forward<T>(result));
    }

    template <typename T>
    inline bool RedisConnection::DECRBY(const std::string &key, int64_t value, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::DECRBY() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context,
                                                              "DECRBY %s %lld", key.c_str(), value));
        return this->parseReply(std::forward<T>(result));
    }

    template <typename T>
    inline bool RedisConnection::HSET(const std::string &key, const std::string &field,
                                      const std::string &value, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::HSET() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "HSET %s %s %s",
                                                              key.c_str(), field.c_str(),
                                                              value.c_str()));
        return this->parseReply(std::forward<T>(result));
    }

    template <typename T>
    inline bool RedisConnection::HMGET(const std::string &key, T &fields, RedisResult &result)
    {
        static_assert(std::is_same<typename std::remove_const<T>::type, std::vector<std::string>>::value ||
                          std::is_same<typename std::remove_const<T>::type, std::initializer_list<std::string>>::value,
                      "RedisConnection::HMGET() only support std::vector<std::string> or std::initializer_list<std::string>");
        std::vector<std::string> _fields = {"HMGET", key};
        _fields.insert(_fields.end(), fields.begin(), fields.end());
        if (!this->execByParams(_fields))
            return false;
        return this->parseReply(result);
    }

    template <typename T>
    inline bool RedisConnection::HMSET(const std::string &key, std::initializer_list<std::string> &&fields, T &values)
    {
        std::vector<std::string> _fields = fields;
        return this->HMSET(key, _fields, values);
    }

    template <typename T>
    inline bool RedisConnection::HMSET(const std::string &key, T &fields, std::vector<std::string> &values)
    {
        static_assert(std::is_same<typename std::remove_const<T>::type, std::vector<std::string>>::value,
                      "RedisConnection::HMSET() only support std::vector<std::string>");
        if (fields.size() != values.size())
            return false;
        std::vector<std::string> _fields = {"HMSET", key};
        int len = fields.size();
        for (int i = 0; i < len; i++)
        {
            _fields.emplace_back(fields[i]);
            _fields.emplace_back(values[i]);
        }
        return this->execByParams(_fields);
    }

    template <typename T>
    inline bool RedisConnection::HINCRBY(const std::string &key, const std::string &field, int64_t value, T &&result)
    {
        static_assert(std::is_same<typename std::remove_reference<T>::type, RedisResult>::value,
                      "RedisConnection::HINCRBY() only support RedisResult");
        this->freeReply();
        this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "HINCRBY %s %s %lld",
                                                              key.c_str(), field.c_str(), value));
        return this->parseReply(std::forward<T>(result));
    }
};

#endif //__MG_REDIS_H__