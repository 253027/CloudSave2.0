#include "redis.h"

#include <sstream>

mg::RedisConnection::~RedisConnection()
{
    if (this->_context)
        redisFree(this->_context);
    this->freeReply();
}

bool mg::RedisConnection::connect(const std::string &ip, uint16_t port, const std::string &password,
                                  int db, int timeout)
{
    this->_ip = ip;
    this->_port = port;
    this->_password = password;
    this->_db = db;

    if (timeout)
    {
        struct timeval time = {timeout, 0};
        this->_context = redisConnectWithTimeout(this->_ip.c_str(), this->_port, time);
    }
    else
        this->_context = redisConnect(this->_ip.c_str(), this->_port);
    if (this->_context->err)
    {
        LOG_ERROR("redis connect error: {}", this->_context->errstr);
        return false;
    }

    if (!this->authenticate(password))
        return false;

    return this->selectDatabase(db);
}

bool mg::RedisConnection::execute(const std::string &command, RedisResult &result)
{
    this->freeReply();
    this->_reply = (redisReply *)redisCommand(this->_context, command.c_str());
    return this->parseReply(result);
}

bool mg::RedisConnection::authenticate(const std::string &password)
{
    if (password.empty())
        return true;
    this->freeReply();
    this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "AUTH %s", password.c_str()));
    if (this->_reply == nullptr || this->_reply->type == REDIS_REPLY_TYPE_ERROR)
    {
        this->freeReply();
        return false;
    }
    return true;
}

bool mg::RedisConnection::selectDatabase(int db)
{
    RedisResult result;
    std::stringstream ss;
    ss << "SELECT " << db;
    return this->execute(ss.str(), result);
}

bool mg::RedisConnection::GET(const std::string &key, RedisResult &result)
{
    this->freeReply();
    this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "GET %s", key.c_str()));
    return this->parseReply(result);
}

bool mg::RedisConnection::SET(const std::string &key, const std::string &value)
{
    this->freeReply();
    this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "SET %s %s",
                                                          key.c_str(), value.c_str()));
    return this->parseReply(_temp);
}

bool mg::RedisConnection::MGET(std::initializer_list<std::string> &&keys, RedisResult &values)
{
    return this->MGET(std::vector<std::string>(keys), values);
}

bool mg::RedisConnection::MSET(std::initializer_list<std::string> &&keys, std::vector<std::string> &values)
{
    return this->MSET(std::vector<std::string>(keys), values);
}

bool mg::RedisConnection::DEL(const std::string &key)
{
    this->freeReply();
    this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "DEL %s",
                                                          key.c_str()));
    return this->parseReply(_temp);
}

bool mg::RedisConnection::HGET(const std::string &key, const std::string &field, RedisResult &result)
{
    this->freeReply();
    this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "HGET %s %s",
                                                          key.c_str(), field.c_str()));
    return this->parseReply(result);
}

bool mg::RedisConnection::HGETALL(const std::string &key, RedisResult &result)
{
    this->freeReply();
    this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "HGETALL %s", key.c_str()));
    return this->parseReply(result);
}

bool mg::RedisConnection::HEXISTS(const std::string &key, const std::string &field, RedisResult &result)
{
    this->freeReply();
    this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "HEXISTS %s %s",
                                                          key.c_str(), field.c_str()));
    return this->parseReply(result);
}

bool mg::RedisConnection::HMGET(const std::string &key, std::initializer_list<std::string> &&fields, RedisResult &result)
{
    return this->HMGET(key, fields, result);
}

bool mg::RedisConnection::HDEL(const std::string &key, const std::string &field)
{
    this->freeReply();
    this->_reply = static_cast<redisReply *>(redisCommand(this->_context, "HDEL %s %s",
                                                          key.c_str(), field.c_str()));
    return this->parseReply(RedisResult());
}

bool mg::RedisConnection::HDEL(const std::string &key, std::vector<std::string> &fields)
{
    this->freeReply();
    std::vector<std::string> _fields = {"HDEL", key};
    _fields.insert(_fields.end(), fields.begin(), fields.end());
    return this->execByParams(_fields);
}

void mg::RedisConnection::freeReply()
{
    if (this->_reply)
    {
        freeReplyObject(this->_reply);
        this->_reply = nullptr;
    }
}

bool mg::RedisConnection::checkReply()
{
    if (!this->_reply)
        return false;
    if (this->_context->err)
    {
        LOG_ERROR("redis execute error: %s", this->_context->errstr);
        return false;
    }
    return true;
}

bool mg::RedisConnection::execByParams(std::vector<std::string> &params)
{
    this->freeReply();

    std::vector<const char *> args;
    std::vector<size_t> lens;
    for (auto &key : params)
    {
        args.push_back(key.c_str());
        lens.push_back(key.size());
    }

    this->_reply = static_cast<redisReply *>(redisCommandArgv(this->_context, params.size(),
                                                              args.data(), lens.data()));
    return this->checkReply();
}