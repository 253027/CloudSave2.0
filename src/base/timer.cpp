#include "timer.h"
#include <assert.h>
#include "log.h"

static std::atomic_int64_t timeSequence{0};
static int64_t generateAndGetID()
{
    return ++timeSequence;
}

mg::Timer::Timer() : _callback(), _expiration(0),
                     _interval(0), _repeat(false),
                     _id(0)
{
    ;
}

mg::Timer::Timer(TimerCallback cb, TimeStamp time, double interval)
    : _callback(cb), _expiration(time),
      _interval(interval), _repeat(interval > 0.0),
      _id(generateAndGetID())
{
    ;
}

mg::Timer::~Timer()
{
    LOG_TRACE("Timer called ~Timer()");
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

const int64_t mg::Timer::getTimerId() const
{
    return this->_id;
}
