#ifndef __MG_EVENT_LOOP_H__
#define __MG_EVENT_LOOP_H__

#include "noncopyable.h"
#include "epoll.h"
#include "time-stamp.h"
#include "timer-queue.h"
#include "current-thread.h"

#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace mg
{
    class Channel;

    class EventLoop : noncopyable
    {
    public:
        EventLoop(const std::string &name);

        ~EventLoop();

        /**
         * @brief 事件循环
         */
        void loop();

        /**
         * @brief 退出事件循环
         */
        void quit();

        /**
         * @brief 唤醒poller避免阻塞在poll函数上
         */
        void wakeup();

        /**
         * @brief 更新channel在epoll中的状态
         */
        void updateChannel(Channel *channel);

        /**
         * @brief 将给定的chanell从对象中移除
         */
        void removeChannel(Channel *channel);

        /**
         * @brief 判断是否已存在epoll中
         * @param channel要检查的channel指针
         */
        bool hasChannel(Channel *channel) const;

        /**
         * @brief eventloop要执行的函数
         */
        void run(std::function<void()> callBack);

        /**
         * @brief 判断执行函数时是否在eventloop所属的线程中执行
         * @return true 在owner线程 false 不在owner线程
         */
        bool isInOwnerThread() { return this->_threadId == currentThread::tid(); }

        /**
         * @brief 非eventloop所属的线程调用此方法，将待执行回调加入eventloop中以待eventloop所在线程执行
         */
        void push(std::function<void()> callBack);

        /**
         * @brief 给定时间执行某个回调函数
         * @param time 给定时间
         * @param callback 待执行回调
         */
        TimerId runAt(TimeStamp time, std::function<void()> callback);

        /**
         * @brief 给定延迟time秒后执行回调
         * @param time 延迟秒数
         */
        TimerId runAfter(double delay, std::function<void()> callback);

        /**
         * @brief 每delay延迟执行一次
         * @param interval 循环执行时间
         */
        TimerId runEvery(double interval, std::function<void()> callback);

        /**
         * @brief 取消定时器任务
         * @param 定时器序列号
         */
        void cancel(TimerId timerId);

        /**
         * @brief 得到当前事件循环的名称
         */
        inline const std::string &getLoopName() const { return this->_name; }

    private:
        // 申请_wakeupFd实例
        int createEventFd();

        /**
         * @brief _wakeupFd的读事件回调函数
         */
        void handleReadCallBack();

        /**
         * @brief 执行其他线程插入的回调函数
         */
        void doPendingFunctions();

        // 时间循环的名称
        std::string _name;
        // 管理的epoll实例
        Epoll _epoller;
        // epoll返回时的时间戳
        TimeStamp _epollReturnTime;
        // 是否处于事件循环中
        std::atomic_bool _looping;
        // 是否退出事件循环
        std::atomic_bool _quit;
        // 是否是时间段类的回调
        std::atomic_bool _callingPendingFunctions;
        // 从epoll获取的活跃的channel集合
        std::vector<Channel *> _channelList;
        // 唤醒_epoller的文件描述符
        int _wakeupFd;
        // 管理_epoller的Channel类
        std::shared_ptr<Channel> _wakeupChannel;
        // 当前线程所属的pid
        const pid_t _threadId;
        // 用于同步线程安全操作（添加或者取出）
        std::mutex _mutex;
        // 存储跨线程操作的回调集合
        std::vector<std::function<void()>> _functions;
        // 定时器队列
        std::shared_ptr<TimerQueue> _timeQueue;
    };
};

#endif //__MG_EVENT_LOOP_H__