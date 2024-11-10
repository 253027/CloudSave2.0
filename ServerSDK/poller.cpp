#include "poller.h"
#include "channel.h"

mg::Poller::Poller(EventLoop *loop) : _loop(loop) {}

bool mg::Poller::hasChannel(Channel *channel) const
{
    return this->_channels.count(channel->fd());
}