#ifndef __MG_THREADPOOL_H__
#define __MG_THREADPOOL_H__

#include "thread.h"

#include <string>
#include <vector>
#include <queue>
#include <condition_variable>
#include <mutex>

namespace mg
{
    class EventLoop;
    class ThreadPool
    {
    public:
        using Task = std::function<void()>;
        /**
         * @brief 线程池构造
         * @param name 线程池名
         * @param queueSize 线程池任务队列大小
         */
        ThreadPool(std::string name = std::string("ThreadPool"), int queueSize = 4096);

        ~ThreadPool();

        /**
         * @brief 启动线程池
         * @param threadNums 要创建的线程数量
         */
        void start(int threadNums);

        /**
         * @brief 如果存在线程则将任务交给线程执行，否则自己执行
         */
        void append(Task task);

        /**
         * @brief 停止线程池
         */
        void stop();

        /**
         * @brief 返回线程池状态
         * @param true 已启动
         * @param false 未启动
         */
        inline bool isStarted() const { return this->_running; };

        /**
         * @brief 返回线程名
         * @return 线程名
         */
        const std::string &getName() const { return this->_name; };

    private:
        /**
         * @brief 创建线程入口函数，线程要执行的操作
         */
        void threadTask();

        mutable std::mutex _mutex;                     // 互斥锁
        std::condition_variable _consumer;             // 消费者等待
        std::condition_variable _productor;            // 生产者等待
        std::string _name;                             // 线程ID
        Task _initialTask;                             // 线程池初始化任务
        std::vector<std::unique_ptr<Thread>> _threads; // 管理线程实例
        std::queue<Task> _taskQueue;                   // 任务队列
        std::atomic_bool _running;                     // 线程池是否处于运行中
        size_t _maxQueueSize;                          // 任务队列最大数量
    };
};

#endif //__MG_THREADPOOL_H__