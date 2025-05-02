#include "timer-queue.h"
#include "event-loop.h"
#include "log.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <assert.h>

static int createTimerFd();
static void readTimerFd(int fd);
static void resetTimerFd(int timefd, mg::TimeStamp expiration);

mg::TimerQueue::TimerQueue(EventLoop *loop)
    : _loop(loop), _timerFd(createTimerFd()),
      _channel(_loop, _timerFd), _list(),
      _activeTimers(), _isCallingExpiredTimers(false),
      _cancleingTimers()
{
    _channel.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    _channel.enableReading();
}

mg::TimerQueue::~TimerQueue()
{
    _channel.disableAllEvents();
    _channel.remove();
    ::close(_timerFd);
    for (auto &x : _list)
        delete x.second;
}

mg::TimerId mg::TimerQueue::addTimer(std::function<void()> callback, TimeStamp time, double interval)
{
    auto timer = new Timer(std::move(callback), time, interval);
    _loop->run(std::bind(&TimerQueue::addTimerInOwnerLoop, this, timer));
    return TimerId(timer, timer->getTimerId());
}

void mg::TimerQueue::cancel(TimerId timerId)
{
    LOG_DEBUG("EventLoop[{}] cancle {}", this->_loop->getLoopName(), timerId._sequence);
    _loop->run(std::bind(&TimerQueue::cancelInOwnerLoop, this, timerId));
}

void mg::TimerQueue::clear()
{
    std::vector<TimerId> memo;
    for (auto &x : _list)
        memo.push_back(TimerId(x.second, x.second->getTimerId()));
    for (auto &x : memo)
        this->cancel(x);
}

void mg::TimerQueue::addTimerInOwnerLoop(Timer *timer)
{
    if (!_loop->isInOwnerThread())
    {
        LOG_ERROR("Timer not added in the owner thread");
        return;
    }
    if (this->insert(timer))
        resetTimerFd(this->_timerFd, timer->expiration());
}

void mg::TimerQueue::handleRead()
{
    TimeStamp now = TimeStamp::now();
    readTimerFd(_timerFd);

    auto expired = this->getExpired(now);
    _isCallingExpiredTimers = true;
    _cancleingTimers.clear();
    for (auto &x : expired)
        x.second->run();
    _isCallingExpiredTimers = false;
    this->reset(expired, now);
}

std::vector<mg::TimerQueue::Entry> mg::TimerQueue::getExpired(TimeStamp time)
{
    std::vector<Entry> expired;
    auto end = _list.upper_bound(Entry(time, nullptr));         // 找到超过给定时间的集合
    std::copy(_list.begin(), end, std::back_inserter(expired)); // 这里不能写expired.end()会导致未定义行为
    _list.erase(_list.begin(), end);
    for (auto &x : expired)
    {
        ActiveTimer timer(x.second, x.second->getTimerId());
        _activeTimers.erase(timer);
    }
    return expired;
}

void mg::TimerQueue::reset(const std::vector<Entry> &expired, TimeStamp now)
{
    TimeStamp nextExpire;
    for (auto &x : expired)
    {
        ActiveTimer timer(x.second, x.second->getTimerId());
        if (x.second->isRepeated() && _cancleingTimers.find(timer) == _cancleingTimers.end())
        {
            x.second->restart(TimeStamp::now());
            this->insert(x.second);
        }
        else
            delete x.second;
    }

    // 重置timerEpoll时间
    if (!_list.empty())
        nextExpire = _list.begin()->second->expiration();
    if (nextExpire.getMircoSecond())
        resetTimerFd(_timerFd, nextExpire);
}

bool mg::TimerQueue::insert(Timer *timer)
{
    bool res = false;
    TimeStamp when = timer->expiration();
    if (!_list.size() || (_list.size() && when < _list.begin()->first))
        res = true;
    _list.insert(Entry(when, timer));
    _activeTimers.insert(std::make_pair(timer, timer->getTimerId()));
    return res;
}

void mg::TimerQueue::cancelInOwnerLoop(TimerId id)
{
    assert(_loop->isInOwnerThread());
    assert(_list.size() == _activeTimers.size());
    ActiveTimer timer(id._timer, id._sequence);
    auto it = _activeTimers.find(timer);
    if (it != _activeTimers.end())
    {
        _list.erase(Entry(it->first->expiration(), it->first));
        delete it->first;
        _activeTimers.erase(it);
    }
    else if (_isCallingExpiredTimers) // 这里是单线程的会执行到这里吗？感觉有问题
        _cancleingTimers.insert(timer);
}

static int createTimerFd()
{
    int timeFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    LOG_DEBUG("create new TimerFd {}", timeFd);
    if (timeFd < 0)
        LOG_ERROR("create new TimerFd failed {}", ::strerror(errno));
    return timeFd;
}

static void readTimerFd(int fd)
{
    uint64_t option = 0;
    int len = ::read(fd, &option, sizeof(option));
    if (len != sizeof(option))
        LOG_ERROR("read TimerFd {} failed", fd);
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
    ts.tv_nsec = static_cast<long>((diff % mg::TimeStamp::_mircoSecondsPerSecond) * 1000);
    newValue.it_value = ts;
    if (::timerfd_settime(timefd, 0, &newValue, &oldValue) < 0)
        LOG_ERROR("Timer {} has error {}", timefd, ::strerror(errno));
}