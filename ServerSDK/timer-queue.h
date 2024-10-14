#ifndef __MG_TIMER_QUEUE_H__
#define __MG_TIMER_QUEUE_H__

#include "timer.h"
#include "channel.h"

#include <set>

namespace mg
{
    class EventLoop;
    class TimerQueue
    {
    public:
        explicit TimerQueue(EventLoop *loop);

        ~TimerQueue();

        void addTimer(std::function<void()> callback, TimeStamp time, double interval);

    private:
        using Entry = std::pair<TimeStamp, std::shared_ptr<Timer>>;
        using TimerList = std::set<Entry>;

        void addTimerInOwnerLoop(const std::shared_ptr<Timer> &timer);

        void handleRead();

        std::vector<Entry> getExpired(TimeStamp time);

        void reset(const std::vector<Entry> &expired, TimeStamp now);

        bool insert(const std::shared_ptr<Timer> &timer);

        EventLoop *_loop;             // 所属的loop
        const int _timerFd;           // linux提供的定时器接口
        Channel _channel;             // 管理_timerFd的channel
        TimerList _list;              // 定时器队列
        bool _isCallingExpiredTimers; // 是否正在获取定时器
    };
};

#endif //__MG_TIMER_QUEUE_H__