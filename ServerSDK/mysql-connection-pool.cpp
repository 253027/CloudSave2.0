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
    _loop = nullptr;
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

    _host = js["ip"];
    _port = js["port"];
    _password = js["password"];
    _username = js["username"];
    _databasename = js["databasename"];
    _maxsize = js["maxsize"];
    _minsize = js["minsize"];
    _timeout = js.contains("timeout") ? ((uint16_t)js["timeout"]) : 0;
    _idletimeout = js.contains("idletimeout") ? ((uint16_t)js["idletimeout"]) : 0;
    _thread.reset(new mg::EventLoopThread(name));
    _loop = _thread->startLoop();
    return true;
}

void mg::MysqlConnectionPool::start()
{
    if (!_thread || _loop == nullptr)
        assert(0);
    {
        std::lock_guard<std::mutex> guard(_mutex);
        addInitial();
    }
    _loop->runEvery(_timeout, std::bind(&MysqlConnectionPool::remove, this));
    _loop->runEvery(_timeout, std::bind(&MysqlConnectionPool::add, this));
}

std::shared_ptr<mg::Mysql> mg::MysqlConnectionPool::get()
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
    std::lock_guard<std::mutex> guard(_mutex);
    while (_queue.size() > _maxsize)
    {
        auto front = _queue.front();
        if (front->getVacantTime().getSeconds() < _idletimeout)
            break;
        _queue.pop();
        SAFE_DELETE(front);
    }
}

void mg::MysqlConnectionPool::add()
{
    std::lock_guard<std::mutex> guard(_mutex);
    if (_queue.size() >= _minsize)
        return;
    addInitial();
    _condition.notify_one();
}

void mg::MysqlConnectionPool::addInitial()
{
    int len = std::min(_minsize, static_cast<uint16_t>(_minsize - _queue.size()));
    for (int i = 0; i < len; i++)
    {
        mg::Mysql *sql = new mg::Mysql();
        if (!sql->connect(_username, _password, _databasename, _host, _port))
        {
            LOG_ERROR("mysql {} connect error", i + 1);
            continue;
        }
        sql->refresh();
        _queue.push(sql);
    }
}
