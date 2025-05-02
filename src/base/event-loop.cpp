#include "event-loop.h"
#include "log.h"
#include "channel.h"

#include <sys/eventfd.h>

__thread mg::EventLoop *t_loopInThisThread = nullptr;
const int waitTime = 10000;

mg::EventLoop::EventLoop(const std::string &name) : _name(name), _epoller(this),
                                                    _epollReturnTime(0), _looping(false),
                                                    _quit(false), _callingPendingFunctions(false),
                                                    _wakeupFd(createEventFd()),
                                                    _wakeupChannel(new Channel(this, _wakeupFd)),
                                                    _threadId(currentThread::tid()), _timeQueue(new TimerQueue(this))
{
    if (t_loopInThisThread)
        LOG_ERROR("EventLoop[{}] existed, repeated create", _name);
    else
        t_loopInThisThread = this;
    _wakeupChannel->setReadCallback(std::bind(&EventLoop::handleReadCallBack, this));
    _wakeupChannel->enableReading();
}

mg::EventLoop::~EventLoop()
{
    this->_wakeupChannel->disableAllEvents();
    this->_wakeupChannel->remove();
    ::close(this->_wakeupFd);
    t_loopInThisThread = nullptr;
}

void mg::EventLoop::loop()
{
    this->_looping = true;
    this->_quit = false;
    LOG_INFO("EventLoop[{}] start looping", this->_name.c_str());
    while (!this->_quit)
    {
        this->_channelList.clear();
        _epollReturnTime = _epoller.poll(_channelList, waitTime);
        for (auto &channel : _channelList)
            channel->handleEvent(_epollReturnTime);
        this->doPendingFunctions();
    }
    this->_looping = false;
}

void mg::EventLoop::quit()
{
    this->_timeQueue->clear();
    this->_quit = true;
    this->wakeup();
    LOG_DEBUG("EventLoop[{}] quit", this->_name.c_str());
}

void mg::EventLoop::wakeup()
{
    uint64_t one = 1;
    int len = ::write(this->_wakeupFd, &one, sizeof(one));
    if (len != sizeof(one))
        LOG_ERROR("EventLoop[{}] wakeup failed", this->_name.c_str());
}

void mg::EventLoop::updateChannel(Channel *channel)
{
    this->_epoller.updateChannel(channel);
}

void mg::EventLoop::removeChannel(Channel *channel)
{
    this->_epoller.removeChannel(channel);
}

bool mg::EventLoop::hasChannel(Channel *channel) const
{
    return this->_epoller.hasChannel(channel);
}

void mg::EventLoop::run(std::function<void()> callBack)
{
    if (this->isInOwnerThread())
        callBack();
    else
        push(callBack);
}

void mg::EventLoop::push(std::function<void()> callBack)
{
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_functions.emplace_back(callBack);
    }
    /*
     *  这里是 _callingPendingFunctions是个小优化，当插入回调时，线程正在执行回调。此时，
     *  队列中的回调不会被遍历到，于是会发生阻塞。
     */
    if (!this->isInOwnerThread() || _callingPendingFunctions)
        this->wakeup();
}

mg::TimerId mg::EventLoop::runAt(TimeStamp time, std::function<void()> callback)
{
    return _timeQueue->addTimer(std::move(callback), time, 0.0);
}

mg::TimerId mg::EventLoop::runAfter(double delay, std::function<void()> callback)
{
    return this->runAt(addTime(TimeStamp::now(), delay), callback);
}

mg::TimerId mg::EventLoop::runEvery(double interval, std::function<void()> callback)
{
    return _timeQueue->addTimer(callback, addTime(TimeStamp::now(), interval), interval);
}

void mg::EventLoop::cancel(TimerId timerId)
{
    _timeQueue->cancel(timerId);
}

int mg::EventLoop::createEventFd()
{
    int ret = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (ret < 0)
    {
        LOG_ERROR("EventLoop[{}] eventFd error: {}", this->_name, strerror(errno));
        return ret;
    }
    LOG_INFO("EventLoop[{}] New wakeup fd {}", this->_name, ret);
    return ret;
}

void mg::EventLoop::handleReadCallBack()
{
    uint64_t one = 1;
    ssize_t len = ::read(this->_wakeupFd, &one, sizeof(one));
    if (len != sizeof(len))
        LOG_ERROR("EventLoop[{}] handleReadCallbak error", this->_name);
}

void mg::EventLoop::doPendingFunctions()
{
    std::vector<std::function<void()>> memo;
    this->_callingPendingFunctions = true;
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_functions.swap(memo);
    }
    for (auto &x : memo)
    {
        x();
        std::function<void()>().swap(x);
    }
    this->_callingPendingFunctions = false;
}
