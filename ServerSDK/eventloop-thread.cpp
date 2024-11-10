#include "eventloop-thread.h"

#include "event-loop.h"

mg::EventLoopThread::EventLoopThread(std::string name, ThreadInitialFunction function)
    : _name(name), _loop(nullptr), _quit(false),
      _thread(std::bind(&EventLoopThread::run, this), name),
      _mutex(), _condition(), _callback(function)
{
    ;
}

mg::EventLoopThread::~EventLoopThread()
{
    this->_quit = true;
    if (this->_loop)
    {
        _loop->quit();
        _thread.join();
    }
}

mg::EventLoop *mg::EventLoopThread::startLoop()
{
    this->_thread.start();
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        while (!this->_loop)
            this->_condition.wait(lock);
        loop = this->_loop;
    }
    return loop;
}

void mg::EventLoopThread::run()
{
    EventLoop loop(this->_name);
    if (this->_callback)
        this->_callback(&loop);
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_loop = &loop;
        this->_condition.notify_one();
    }
    // 这里执行了底层的事件循环，如果还能往下走说明事件循环停止了，此时释放掉_loop
    this->_loop->loop();

    std::lock_guard<std::mutex> guard(this->_mutex);
    this->_loop = nullptr;
}
