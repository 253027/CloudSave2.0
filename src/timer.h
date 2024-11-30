#ifndef __MG_TIMER_H__
#define __MG_TIMER_H__

#include "noncopyable.h"
#include "time-stamp.h"

#include <functional>

namespace mg
{
    class Timer : noncopyable
    {
    public:
        using TimerCallback = std::function<void()>;

        Timer();

        Timer(TimerCallback cb, TimeStamp time, double interval);

        ~Timer();

        /**
         * @brief 执行定时器保存的回调函数
         */
        void run();

        /**
         * @brief 返回超时时间
         */
        const TimeStamp &expiration() const;

        /**
         * @brief 定时器是否可重复执行
         * @return true 可以 false 不可以
         */
        bool isRepeated();

        /**
         * @brief 重启定时器，修改下次超时时间
         */
        void restart(TimeStamp now);

        /**
         * @brief 返回Timer类的唯一标识符
         */
        const int64_t getTimerId() const;

    private:
        const TimerCallback _callback; // 超时后执行的回调
        TimeStamp _expiration;         // 超时时间
        const double _interval;        // 超时时间间隔
        const bool _repeat;            // 是否可复用
        const int64_t _id;             // 标识timer的唯一id
    };
};

#endif //__MG_TIMER_H__