#ifndef __MG_EVENTLOOP_THREADPOOL_H__
#define __MG_EVENTLOOP_THREADPOOL_H__

#include "eventloop-thread.h"

#include <functional>
#include <vector>

namespace mg
{
    class EventLoop;
    class EventLoopThreadPool
    {
    public:
        using ThreadInitialCallback = std::function<void(EventLoop *)>;

        EventLoopThreadPool(EventLoop *baseLoop, const std::string &name);

        ~EventLoopThreadPool();

        /**
         * @brief 设置线程数量
         */
        void setThreadNums(int nums);

        /**
         * @brief 启动线程池
         */
        void start(ThreadInitialCallback callBack = ThreadInitialCallback());

        /**
         * @brief 以轮询方式得到下一个eventloop实例
         * @return 返回指向实例的指针
         */
        EventLoop *getNextLoop();

        /**
         * @brief 得到包含所有实例的指针
         * @return 实例指针集合
         */
        std::vector<EventLoop *> getAllEventLoops();

        inline bool isStarted() { return this->_started; };

    private:
        EventLoop *_baseloop;                                   // 主线程的事件循环
        std::string _name;                                      // 线程池名字
        bool _started;                                          // 标志是否开启
        int _threadNums;                                        // 线程数
        std::vector<std::unique_ptr<EventLoopThread>> _threads; // 线程集合
        std::vector<EventLoop *> _loops;                        // 事件循环集合

        struct pair
        {
            int first = 0;
            int second = 0;
        };
        std::atomic<pair> _next; // 下一个轮询到的时间循环下标
    };
};

#endif //__MG_EVENTLOOP_THREADPOOL_H__