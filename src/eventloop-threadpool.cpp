#include "eventloop-threadpool.h"

mg::EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &name)
    : _baseloop(baseLoop), _name(name), _started(false),
      _threadNums(0), _next(0)
{
    ;
}

mg::EventLoopThreadPool::~EventLoopThreadPool()
{
    ;
}

void mg::EventLoopThreadPool::setThreadNums(int nums)
{
    if (nums < 0)
        return;
    this->_threadNums = nums;
}

void mg::EventLoopThreadPool::start(ThreadInitialCallback callBack)
{
    this->_started = true;
    if (!this->_threadNums && callBack)
    {
        callBack(this->_baseloop);
        return;
    }

    for (int i = 0; i < this->_threadNums; i++)
    {
        char buf[128] = {0};
        snprintf(buf, sizeof(buf) - 1, "%s-%d", this->_name.c_str(), i);
        EventLoopThread *temp = new EventLoopThread(buf, callBack);
        this->_threads.push_back(std::unique_ptr<EventLoopThread>(temp));
        this->_loops.push_back(temp->startLoop());
    }
}

mg::EventLoop *mg::EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = this->_baseloop;
    if (!this->_loops.empty())
    {
        loop = this->_loops[_next];
        _next = (_next + 1) % this->_loops.size();
    }
    return loop;
}

std::vector<mg::EventLoop *> mg::EventLoopThreadPool::getAllEventLoops()
{
    if (_loops.empty())
        return {_baseloop};
    return _loops;
}
