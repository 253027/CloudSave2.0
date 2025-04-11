#ifndef __MG_TIMER_QUEUE_H__
#define __MG_TIMER_QUEUE_H__

#include "timer.h"
#include "channel.h"
#include "timer-id.h"

#include <set>
#include <utility>
#include <atomic>
#include <vector>

namespace mg
{
    class EventLoop;
    class TimerQueue
    {
    public:
        explicit TimerQueue(EventLoop *loop);

        ~TimerQueue();

        /**
         * @brief 添加定时器任务
         * @return 定时器ID
         */
        TimerId addTimer(std::function<void()> callback, TimeStamp time, double interval);

        /**
         * @brief 取消定时器
         * @param 取消定时器ID
         */
        void cancel(TimerId timerId);

        /**
         * @brief 清除所有定时器
         */
        void clear();

    private:
        using Entry = std::pair<TimeStamp, Timer *>;
        using TimerList = std::set<Entry>;
        using ActiveTimer = std::pair<Timer *, int64_t>;
        using ActiveTimerSet = std::set<ActiveTimer>;

        void addTimerInOwnerLoop(Timer *timer);

        void handleRead();

        std::vector<Entry> getExpired(TimeStamp time);

        void reset(const std::vector<Entry> &expired, TimeStamp now);

        bool insert(Timer *timer);

        void cancelInOwnerLoop(TimerId id);

        EventLoop *_loop;                         // 所属的loop
        const int _timerFd;                       // linux提供的定时器接口
        Channel _channel;                         // 管理_timerFd的channel
        TimerList _list;                          // 定时器队列
        ActiveTimerSet _activeTimers;             // 已销毁的连接处于活跃的定时器事件集合
        std::atomic_bool _isCallingExpiredTimers; // 是否处于执行定时器事件中
        ActiveTimerSet _cancleingTimers;          // 待销毁的定时器事件集合
    };
    /**
     * @date 2024-10-19 21:42
     * @brief
     *  tips:
     *      在完成这个类时，编译后运行出现了问题，报错为在event-loop的构造函数中初始化timer-queue时内存访问越界了
     *      原因是在于make编译时只对产生变动的文件重新编译，如果存在某个文件的头文件下某个类的成员发生变化，引用这个头文件的*.cpp
     *      文件并不会重新编译。在本例中timer-queue.h添加了_cancleingTimers成员，而原来的event-loop.o文件内TimerQueue类中并没有
     *      这个成员于是就发生了上述情况.
     */
};

#endif //__MG_TIMER_QUEUE_H__