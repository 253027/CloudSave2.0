#include "timer.h"
#include <assert.h>

mg::Timer::Timer(TimerCallback cb, TimeStamp time, double interval)
    : _callback(cb), _expiration(time), _interval(interval), _repeat(interval > 0.0)
{
    ;
}

void mg::Timer::run()
{
    if (!_callback)
        assert(0);
    _callback();
}

const mg::TimeStamp &mg::Timer::expiration() const
{
    return _expiration;
}

bool mg::Timer::isRepeated()
{
    return _repeat;
}

void mg::Timer::restart(TimeStamp now)
{
    if (_repeat)
        _expiration = mg::addTime(now, _interval);
    else
        _expiration = mg::TimeStamp(0);
}
