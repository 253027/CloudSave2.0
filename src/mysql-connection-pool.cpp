#include "mysql-connection-pool.h"
#include "json.hpp"
#include "event-loop.h"
#include "mysql.h"
#include "eventloop-thread.h"
#include "log.h"
#include "macros.h"
#include <fstream>

using json = nlohmann::json;

mg::MysqlConnectionPool::MysqlConnectionPool() : _host(), _username(), _password(),
                                                 _databasename(), _port(0), _maxsize(0),
                                                 _minsize(0), _timeout(0), _idletimeout(0),
                                                 _loop(nullptr)
{
    ;
}

mg::MysqlConnectionPool::~MysqlConnectionPool()
{
    _loop = nullptr; // loop的声明周期不由MysqlConnectionPool管理
#ifdef _DEBUG
    LOG_DEBUG("~MysqlConnectionPool() called");
#endif
}

bool mg::MysqlConnectionPool::initial(const std::string &configPath, const std::string &name)
{
    json js;
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        LOG_ERROR("{} open file faild", name);
        return false;
    }
    try
    {
        file >> js;
    }
    catch (const json::parse_error &e)
    {
        LOG_ERROR("{} json parse error: {}", e.what());
    }
    file.close();

    _host = js.value("ip", "localhost");
    _port = js.value("port", 0);
    _password = js.value("password", "");
    _username = js.value("username", "");
    _databasename = js.value("databasename", "");
    _maxsize = js.value("maxsize", 1);
    _minsize = js.value("minsize", 1);
    _timeout = js.value("timeout", 0);
    _idletimeout = js.value("idletimeout", 0);
    _thread.reset(new mg::EventLoopThread(name));
    _loop = _thread->startLoop();
    return true;
}

bool mg::MysqlConnectionPool::start(int keeplive)
{
    if (!_thread || _loop == nullptr)
        assert(0);
    {
        std::lock_guard<std::mutex> guard(_mutex);
        if (!addInitial())
            return false;
    }
    _loop->runEvery(_timeout, std::bind(&MysqlConnectionPool::remove, this));
    _loop->runEvery(_timeout, std::bind(&MysqlConnectionPool::add, this));
    if (keeplive)
        _loop->runEvery(keeplive, std::bind(&MysqlConnectionPool::keepAlive, this));
    return true;
}

void mg::MysqlConnectionPool::quit()
{
    _loop->quit();
}

std::shared_ptr<mg::Mysql> mg::MysqlConnectionPool::getHandle()
{
    // 不能在定时器线程中执行此函数会导致死锁
    assert(!_loop->isInOwnerThread());

    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty() && _condition.wait_for(lock, std::chrono::milliseconds(500)) == std::cv_status::timeout)
        return nullptr;

    std::shared_ptr<Mysql> res(_queue.front(), [this](mg::Mysql *connection)
                               {
                                   std::lock_guard<std::mutex> guard(this->_mutex);
                                   connection->refresh();
                                   this->_queue.push(connection);
                                   this->_condition.notify_one(); // 通知等待的线程有新连接可用
                               });

    _queue.pop();
    return res;
}

void mg::MysqlConnectionPool::remove()
{
    LOG_TRACE("mysql remove called");
    std::lock_guard<std::mutex> guard(_mutex);
    while (_queue.size() > _maxsize)
    {
        auto front = _queue.front();
        if (front->getVacantTime().getSeconds() < _idletimeout)
            break;
        _queue.pop();
        LOG_TRACE("mysql remove {}", (void *)front);
        SAFE_DELETE(front);
    }
}

void mg::MysqlConnectionPool::add()
{
    LOG_TRACE("mysql add called");
    std::lock_guard<std::mutex> guard(_mutex);
    if (_queue.size() >= _minsize)
        return;
    addInitial();
    _condition.notify_one();
}

bool mg::MysqlConnectionPool::addInitial()
{
    int len = std::min(_minsize, static_cast<uint16_t>(_minsize - _queue.size()));
    for (int i = 0; i < len; i++)
    {
        mg::Mysql *sql = new mg::Mysql();
        if (!sql->connect(_username, _password, _databasename, _host, _port))
        {
            LOG_ERROR("mysql {} connect error", i + 1);
            return false;
        }
        sql->refresh();
        LOG_TRACE("mysql add {}", (void *)sql);
        _queue.push(sql);
    }
    return true;
}

void mg::MysqlConnectionPool::keepAlive()
{
    std::string sql = "select now()";
    std::lock_guard<std::mutex> guard(_mutex);
    std::queue<Mysql *> temp;
    while (!_queue.empty())
    {
        auto front = _queue.front();
        _queue.pop();
        temp.push(front);
        front->query(sql);
        LOG_TRACE("mysql keepalive {}", (void *)front);
    }
    temp.swap(_queue);
}