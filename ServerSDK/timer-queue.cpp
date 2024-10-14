#include "timer-queue.h"
#include "event-loop.h"
#include "log.h"

#include <sys/timerfd.h>
#include <unistd.h>

static int createTimerFd();
static void readTimerFd(int fd);
static void resetTimerFd(int timefd, mg::TimeStamp expiration);

mg::TimerQueue::TimerQueue(EventLoop *loop)
    : _loop(loop), _timerFd(createTimerFd()),
      _channel(_loop, _timerFd), _list()
{
    _channel.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    _channel.enableReading();
}

mg::TimerQueue::~TimerQueue()
{
    _channel.disableAllEvents();
    _channel.remove();
    ::close(_timerFd);
}

void mg::TimerQueue::addTimer(std::function<void()> callback, TimeStamp time, double interval)
{
    auto timer = std::make_shared<Timer>(std::move(callback), time, interval);
    _loop->run(std::bind(&TimerQueue::addTimerInOwnerLoop, this, timer));
}

void mg::TimerQueue::addTimerInOwnerLoop(const std::shared_ptr<Timer> &timer)
{
    if (this->insert(timer))
        resetTimerFd(this->_timerFd, timer->expiration());
}

void mg::TimerQueue::handleRead()
{
    TimeStamp now = TimeStamp::now();
    readTimerFd(_timerFd);

    auto expired = this->getExpired(now);
    _isCallingExpiredTimers = true;
    for (auto &x : expired)
        x.second->run();
    _isCallingExpiredTimers = false;
}

std::vector<mg::TimerQueue::Entry> mg::TimerQueue::getExpired(TimeStamp time)
{
    std::vector<Entry> expired;
    auto end = _list.upper_bound(Entry(time, reinterpret_cast<Timer *>(UINTPTR_MAX))); // 找到超过给定时间的集合
    std::copy(_list.begin(), end, std::back_inserter(expired));                        // 这里不能写expired.end()会导致未定义行为
    _list.erase(_list.begin(), end);
    return expired;
}

void mg::TimerQueue::reset(const std::vector<Entry> &expired, TimeStamp now)
{
    TimeStamp nextExpire;
    for (auto &x : expired)
    {
        if (x.second->isRepeated())
        {
            x.second->restart(TimeStamp::now());
            this->insert(x.second);
        }
    }

    if (!_list.empty())
        nextExpire = _list.begin()->second->expiration();

    if (nextExpire.getMircoSecond())
        resetTimerFd(_timerFd, nextExpire);
}

bool mg::TimerQueue::insert(const std::shared_ptr<Timer> &timer)
{
    bool res = false;
    TimeStamp when = timer->expiration();
    if (_list.size() && when < _list.begin()->first)
        res = true;
    res = _list.insert(Entry(when, timer)).second && res;
    return res;
}

static int createTimerFd()
{
    int timeFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    LOG_DEBUG("Create TimerFd[{}]", timeFd);
    if (timeFd < 0)
        LOG_ERROR("Create TimerFd failed {}", ::strerror(errno));
    return timeFd;
}

static void readTimerFd(int fd)
{
    uint64_t option = 0;
    int len = ::read(fd, &option, sizeof(option));
    if (len != sizeof(option))
        LOG_ERROR("Timer[{}] readTimerFd failed", fd);
}

static void resetTimerFd(int timefd, mg::TimeStamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    ::bzero(&newValue, sizeof(newValue));
    ::bzero(&oldValue, sizeof(oldValue));

    int64_t diff = expiration.getMircoSecond() - mg::TimeStamp::now().getMircoSecond();
    diff = std::max(100L, diff);

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(diff / mg::TimeStamp::_mircoSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(diff % mg::TimeStamp::_mircoSecondsPerSecond * 1000);
    newValue.it_value = ts;
    if (::timerfd_settime(timefd, 0, &newValue, &oldValue))
        LOG_ERROR("Timer[{}] resetTimerFd failed", timefd);
}