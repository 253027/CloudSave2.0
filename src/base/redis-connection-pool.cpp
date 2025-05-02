#include "redis-connection-pool.h"
#include "eventloop-thread.h"
#include "event-loop.h"
#include "log.h"
#include "macros.h"

#include "../common/common-macro.h"
#include "json.hpp"
#include <fstream>

mg::RedisConnectionPool::RedisConnectionPool(mg::EventLoop *loop, const std::string &name)
    : _loop(loop), _name(name), _db(0), _port(0), _maxsize(0), _minsize(0),
      _totalsize(0), _timeout(0), _idletimeout(), _keepalive(false)
{
    ;
}

mg::RedisConnectionPool::~RedisConnectionPool()
{
    /**
     * FIXME: Before destruction, connections in the queue
     *        may have been allocated but not fully released
     */
    while (!_queue.empty())
    {
        SAFE_DELETE(_queue.front());
        _queue.pop_front();
    }
}

bool mg::RedisConnectionPool::initial(const std::string &configPath, const std::string &name)
{
    PARSE_JSON_FILE(js, configPath);
    return this->initial(js, name);
}

bool mg::RedisConnectionPool::initial(nlohmann::json &config, const std::string &name)
{
    _host = config.value("ip", "localhost");
    _port = config.value("port", 0);
    _password = config.value("password", "");
    _maxsize = config.value("maxsize", 1);
    _minsize = config.value("minsize", 1);
    _timeout = config.value("timeout", 0);
    _idletimeout = config.value("idletimeout", 0);
    if (this->_loop == nullptr)
    {
        _thread.reset(new mg::EventLoopThread(name));
        _loop = _thread->startLoop();
    }
    return true;
}

bool mg::RedisConnectionPool::start()
{
    assert(this->_loop != nullptr && "redis start failed");
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!this->addInitial())
            return false;
    }

    this->_loop->runEvery(this->_timeout, std::bind(&RedisConnectionPool::add, this));
    this->_loop->runEvery(this->_timeout, std::bind(&RedisConnectionPool::remove, this));

    if (this->_keepalive)
    {
        this->_loop->runEvery(this->_idletimeout, [this]()
                              {
                                  std::lock_guard<std::mutex> guard(this->_mutex);
                                  mg::RedisResult result;
                                  int size = this->_queue.size();
                                  for (int i = 0; i < size; i++)
                                  {
                                      auto con = this->_queue.front();
                                      this->_queue.pop_front();
                                      result.reset();
                                      con->execute("PING", result);
                                      this->_queue.push_back(con);
                                      std::string resultStr = result;
                                      if (resultStr == "PONG")
                                      {
                                          con->refresh();
                                          LOG_TRACE("redis {} keepalive {}", this->_name, (void *)con);
                                      }
                                      else
                                          LOG_ERROR("redis {} keepalive error {}", this->_name, (void *)con);
                                  } //
                              });
    }
    LOG_INFO("redis pool {} start success", this->_name);
    return true;
}

void mg::RedisConnectionPool::quit()
{
    if (this->_thread)
        this->_loop->quit();
}

std::shared_ptr<mg::RedisConnection> mg::RedisConnectionPool::getHandle()
{
    assert(!_loop->isInOwnerThread());

    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty() && _condition.wait_for(lock, std::chrono::milliseconds(500)) == std::cv_status::timeout)
        return nullptr;

    std::shared_ptr<RedisConnection> res(_queue.front(), [this](mg::RedisConnection *connection)
                                         {
                                             std::lock_guard<std::mutex> guard(this->_mutex);
                                             connection->refresh();
                                             this->_queue.push_back(connection);
                                             this->_condition.notify_one(); //
                                         });

    _queue.pop_front();
    return res;
}

bool mg::RedisConnectionPool::addInitial()
{
    int len = std::min(_minsize, static_cast<uint16_t>(_minsize - _queue.size()));
    for (int i = 0; i < len; i++)
    {
        mg::RedisConnection *redis = new mg::RedisConnection();
        if (!redis->connect(this->_host, this->_port, this->_password, this->_db, this->_timeout))
            return false;
        this->_queue.push_back(redis);
    }
    return true;
}

void mg::RedisConnectionPool::add()
{
    LOG_TRACE("redis {} add called", this->_name);
    std::lock_guard<std::mutex> guard(_mutex);
    if (!_queue.empty() || this->_totalsize > this->_maxsize)
        return;
    addInitial();
    _condition.notify_one();
}

void mg::RedisConnectionPool::remove()
{
    LOG_TRACE("redis {} remove called", this->_name);
    std::lock_guard<std::mutex> guard(_mutex);
    while (_queue.size() > _minsize)
    {
        auto front = _queue.front();
        if (front->getVacantTime().getSeconds() < _idletimeout)
            break;
        _queue.pop_front();
        LOG_TRACE("redis {} remove {}", this->_name, (void *)front);
        SAFE_DELETE(front);
        this->_totalsize--;
    }
}

mg::RedisPoolManager::~RedisPoolManager()
{
    this->quit();
}

bool mg::RedisPoolManager::initial(const std::string &configPath, EventLoop *loop)
{
    PARSE_JSON_FILE(config, configPath);
    return this->initial(config, loop);
}

bool mg::RedisPoolManager::initial(nlohmann::json &config, EventLoop *loop)
{
    this->_loop = loop;
    if (this->_loop == nullptr)
    {
        _thread.reset(new mg::EventLoopThread("RedisPoolManager"));
        _loop = _thread->startLoop();
    }
    for (auto &item : config.items())
    {
        if (!item.value().is_object())
            continue;
        auto pool = std::make_shared<RedisConnectionPool>(this->_loop, item.key());
        if (!pool->initial(item.value(), item.key()))
            assert(0 && "redis pool initial failed");
        pool->setKeepAlive();
        if (!pool->start())
            assert(0 && "redis pool start failed");
        this->_pools[item.key()] = pool;
    }
    return true;
}

void mg::RedisPoolManager::quit()
{
    for (auto &pool : _pools)
        pool.second->quit();
}

std::shared_ptr<mg::RedisConnectionPool> mg::RedisPoolManager::getPool(const std::string &poolName)
{
    auto it = this->_pools.find(poolName);
    if (it == this->_pools.end())
        return nullptr;
    return it->second;
}

std::shared_ptr<mg::RedisConnection> mg::RedisPoolManager::getHandle(const std::string &poolName)
{
    auto pool = this->getPool(poolName);
    if (!pool)
        return nullptr;
    return pool->getHandle();
}