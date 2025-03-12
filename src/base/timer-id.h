#ifndef __MG_TIMER_ID_H__
#define __MG_TIMER_ID_H__

#include "noncopyable.h"

#include <cstdint>

namespace mg
{
    class Timer;
    class TimerId
    {
    public:
        TimerId();

        TimerId(Timer *timer, int64_t seq);

        friend class TimerQueue;
        friend class TcpConnection;

    private:
        Timer *_timer;
        int64_t _sequence;
    };

};

#endif //__MG_TIMER_ID_H__