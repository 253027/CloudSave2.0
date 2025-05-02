#include "channel.h"
#include "event-loop.h"
#include "log.h"

#include <sys/epoll.h>

namespace mg
{
    const int readEvent = EPOLLIN | EPOLLPRI;
    const int writeEvent = EPOLLOUT;
}

mg::Channel::Channel(EventLoop *loop, int fd) : _loop(loop), _fd(fd),
                                                _events(0), _activeEvents(0),
                                                _index(newChannel), _tied(false),
                                                _handleEvent(false)
{
    ;
}

mg::Channel::~Channel()
{
    assert(!_handleEvent);
    if (_loop->isInOwnerThread())
    {
        assert(!_loop->hasChannel(this));
    }
}

void mg::Channel::handleEvent(TimeStamp receiveTime)
{
    std::shared_ptr<void> guard = this->_tie.lock();
    if (!this->_tied || guard)
        handleEventWithGuard(receiveTime);
}

void mg::Channel::setReadCallback(ReadEventCallback readBack)
{
    this->_readCallback = std::move(readBack);
}

void mg::Channel::setWriteCallback(EventCallback callBack)
{
    this->_writeCallback = std::move(callBack);
}

void mg::Channel::setCloseCallback(EventCallback callBack)
{
    this->_closeCallback = std::move(callBack);
}

void mg::Channel::setErrorCallback(EventCallback callBack)
{
    this->_errorCallback = std::move(callBack);
}

int mg::Channel::fd() const
{
    return this->_fd;
}

int mg::Channel::events() const
{
    return this->_events;
}

void mg::Channel::enableWriting()
{
    this->_events |= writeEvent;
    this->update();
}

void mg::Channel::disableWriting()
{
    this->_events &= ~writeEvent;
    this->update();
}

void mg::Channel::enableReading()
{
    this->_events |= readEvent;
    this->update();
}

void mg::Channel::disableReading()
{
    this->_events &= ~readEvent;
    this->update();
}

void mg::Channel::setActiveEvents(int activeEvents)
{
    this->_activeEvents = activeEvents;
}

void mg::Channel::setIndex(int index)
{
    this->_index = index;
}

int mg::Channel::index() const
{
    return this->_index;
}

bool mg::Channel::isNoneEvent() const
{
    return this->_events == 0;
}

bool mg::Channel::isWriting() const
{
    return this->_events & writeEvent;
}

bool mg::Channel::isReading() const
{
    return this->_events & readEvent;
}

void mg::Channel::disableAllEvents()
{
    LOG_TRACE("EventLoop[{}] Channel {} disableAllEvents",
              this->_loop->getLoopName(), this->_fd);
    this->_events = 0;
    this->update();
}

void mg::Channel::remove()
{
    this->_loop->removeChannel(this);
}

void mg::Channel::tie(const std::shared_ptr<void> &object)
{
    this->_tie = object;
    this->_tied = true;
}

mg::EventLoop *mg::Channel::ownerLoop()
{
    return this->_loop;
}

void mg::Channel::update()
{
    if (!this->_loop)
    {
        LOG_ERROR("EventLoop[{}] Channel {} call update error is nullptr",
                  this->_loop->getLoopName(), this->_fd);
        return;
    }
    this->_loop->updateChannel(this);
}

void mg::Channel::handleEventWithGuard(TimeStamp time)
{
    _handleEvent = true;
    // 对方关闭连接会触发EPOLLHUP
    if ((this->_activeEvents & EPOLLHUP) && !(this->_activeEvents & EPOLLIN))
    {
        LOG_TRACE("EventLoop[{}] Channel {} closed event 0x{:x}",
                  this->_loop->getLoopName(), this->_fd, this->_activeEvents);
        if (_closeCallback)
            _closeCallback();
    }

    if (this->_activeEvents & EPOLLERR)
    {
        LOG_TRACE("EventLoop[{}] Channel {} error event 0x{:x}",
                  this->_loop->getLoopName(), this->_fd, this->_activeEvents);
        if (_errorCallback)
            _errorCallback();
    }

    // EPOLLIN表示普通数据和优先数据可读，EPOLLPRI表示高优先数据可读，EPOLLRDHUP表示TCP连接对方关闭或者对方关闭写端
    if (this->_activeEvents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        LOG_TRACE("EventLoop[{}] Channel {} read event 0x{:x}",
                  this->_loop->getLoopName(), this->_fd, this->_activeEvents);
        if (_readCallback)
            _readCallback(time);
    }

    // 写事件发生，处理可写事件
    if (this->_activeEvents & EPOLLOUT)
    {
        LOG_TRACE("EventLoop[{}] Channel {} write event 0x{:x}",
                  this->_loop->getLoopName(), this->_fd, this->_activeEvents);
        if (_writeCallback)
            _writeCallback();
    }

    _handleEvent = false;
}
