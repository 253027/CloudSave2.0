#include "time-stamp.h"

mg::TimeStamp::TimeStamp() : _microsecond(0) {}

mg::TimeStamp::TimeStamp(int64_t mircosecond) : _microsecond(mircosecond) {}

mg::TimeStamp mg::TimeStamp::now()
{
    struct timeval time;
    ::gettimeofday(&time, nullptr);
    int64_t seconds = time.tv_sec;
    return TimeStamp(seconds * _mircoSecondsPerSecond + time.tv_usec);
}

std::string mg::TimeStamp::toString() const
{
    std::string ret;
    ret = std::to_string(_microsecond / _mircoSecondsPerSecond);
    ret += ".";
    ret += std::to_string(_microsecond % _mircoSecondsPerSecond);
    return ret;
}

std::string mg::TimeStamp::toFormatString(bool showMileSecond, bool needUTC) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(_microsecond / _mircoSecondsPerSecond);
    struct tm tm_time;

    if (needUTC)
        gmtime_r(&seconds, &tm_time);
    else
        localtime_r(&seconds, &tm_time);

    int len = snprintf(buf, sizeof(buf) - 1, "%4d%02d%02d-%02d:%02d:%02d",
                       tm_time.tm_year + 1900, tm_time.tm_mon + 1,
                       tm_time.tm_mday, tm_time.tm_hour,
                       tm_time.tm_min, tm_time.tm_sec);
    if (showMileSecond)
        snprintf(buf + len, sizeof(buf) - len - 1, ".%06d", static_cast<int>(_microsecond % _mircoSecondsPerSecond));
    return buf;
}

mg::TimeStamp mg::addTime(mg::TimeStamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * mg::TimeStamp::_mircoSecondsPerSecond);
    return mg::TimeStamp(timestamp._microsecond + delta);
}