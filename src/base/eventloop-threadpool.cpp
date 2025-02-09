#include "eventloop-threadpool.h"

mg::EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &name)
    : _baseloop(baseLoop), _name(name), _started(false),
      _threadNums(0)
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
#if 0
        //std::atomic_int _next;
        int next = _next.fetch_add(1) % this->_loops.size();
        loop = this->_loops[next];
#endif

#if 1
        pair current, next;
        do
        {
            current = _next.load(std::memory_order_acquire);
            next.first = (current.first + 1) % this->_loops.size();
            next.second = current.second + 1;
            loop = this->_loops[next.first];
        } while (!_next.compare_exchange_weak(current, next, std::memory_order_acquire));

#endif
    }
    return loop;
}

std::vector<mg::EventLoop *> mg::EventLoopThreadPool::getAllEventLoops()
{
    if (_loops.empty())
        return {_baseloop};
    return _loops;
}
