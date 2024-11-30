#include "thread.h"
#include "current-thread.h"

#include <semaphore.h>

std::atomic<int32_t> mg::Thread::_threadIndex(0);

mg::Thread::Thread(ThreadFunction cb, const std::string &name)
    : _start(false), _join(false), _tid(0),
      _name(name), _function(std::move(cb))
{
    setDefaultName();
}

mg::Thread::~Thread()
{
    if (_start && !_join)
        _thread->detach();
}

void mg::Thread::start()
{
    _start = true;
    sem_t semephore;
    sem_init(&semephore, false, 0);
    _thread = std::shared_ptr<std::thread>(new std::thread(
        [&]()
        {
            this->_tid = currentThread::tid();
            sem_post(&semephore);
            this->_function(); // 执行入口函数
        }));
    sem_wait(&semephore);
}

void mg::Thread::join()
{
    this->_join = true;
    _thread->join();
}

void mg::Thread::setDefaultName()
{
    int num = _threadIndex++;
    if (this->_name.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread[%d]", num);
        this->_name = buf;
    }
}
