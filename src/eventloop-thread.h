#ifndef __MG_EVENTLOOP_THREAD_H__
#define __MG_EVENTLOOP_THREAD_H__

#include "thread.h"

#include <functional>
#include <mutex>
#include <condition_variable>
#include <semaphore>

namespace mg
{
    class EventLoop;
    class EventLoopThread
    {
    public:
        using ThreadInitialFunction = std::function<void(EventLoop *)>;

        EventLoopThread(std::string name = std::string(), ThreadInitialFunction function = ThreadInitialFunction());

        ~EventLoopThread();

        EventLoop *startLoop();

        void run();

    private:
        EventLoop *_loop; // 线程所属的事件循环
        std::string _name;
        bool _quit;
        Thread _thread;
        std::mutex _mutex;
        std::condition_variable _condition;
        ThreadInitialFunction _callback;
    };
};

#endif //__MG_EVENTLOOP_THREAD_H__