#include "redis-connection-pool.h"
#include "eventloop-thread.h"
#include "event-loop.h"
#include "log.h"
#include "macros.h"

#include "../common/common-macro.h"
#include "json.hpp"
#include <fstream>

mg::RedisConnectionPool::RedisConnectionPool(mg::EventLoop *loop)
    : _loop(loop), _db(0), _port(0), _maxsize(0), _minsize(0),
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
        this->_loop->runEvery(this->_timeout, [this]()
                              {
                                  std::lock_guard<std::mutex> guard(_mutex);
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
                                          LOG_TRACE("redis keepalive {}", (void *)con);
                                      }
                                      else
                                          LOG_ERROR("redis keepalive error {}", (void *)con);
                                  } //
                              });
    }
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
    LOG_TRACE("redis add called");
    std::lock_guard<std::mutex> guard(_mutex);
    if (!_queue.empty() || this->_totalsize > this->_maxsize)
        return;
    addInitial();
    _condition.notify_one();
}

void mg::RedisConnectionPool::remove()
{
    LOG_TRACE("redis remove called");
    std::lock_guard<std::mutex> guard(_mutex);
    while (_queue.size() > _minsize)
    {
        auto front = _queue.front();
        if (front->getVacantTime().getSeconds() < _idletimeout)
            break;
        _queue.pop_front();
        LOG_TRACE("redis remove {}", (void *)front);
        SAFE_DELETE(front);
        this->_totalsize--;
    }
}