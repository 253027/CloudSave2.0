#include "epoll.h"
#include "log.h"
#include "channel.h"

using namespace mg;

Epoll::Epoll(EventLoop *loop) : Poller(loop)
{
    this->_epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
    if (this->_epoll_fd < 0)
        LOG_ERROR("Epoll create failed");
    else
        LOG_INFO("New epoll_fd is created: {}", this->_epoll_fd);
    _events.resize(_events_initial_size);
}

Epoll::~Epoll()
{
    ::close(this->_epoll_fd);
}

TimeStamp Epoll::poll(std::vector<Channel *> &channelList, int timeout)
{
    LOG_INFO("epoll[{}] has {} channels", this->_epoll_fd, this->_channels.size());
    int nums = ::epoll_wait(this->_epoll_fd, _events.data(), static_cast<int>(_events.size()), timeout);
    if (nums >= 0)
    {
        LOG_INFO("epoll[{}] receive {} evetns", this->_epoll_fd, nums);
        if (nums == _events.size())
            _events.resize(nums << 1);
        this->fillActiveChannels(nums, channelList);
    }
    else if (nums < 0 && errno != EINTR)
        LOG_ERROR("epoll[{}] receive error {}", this->_epoll_fd, errno);
    return TimeStamp::now();
}

void mg::Epoll::updateChannel(Channel *channel)
{
    const int index = channel->index();
    if (index == newChannel || index == deletedChannel)
    {
        int fd = channel->fd();
        if (index == newChannel)
            this->_channels[fd] = channel;
        channel->setIndex(addedChannel);
        this->update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        if (channel->isNoneEvent())
        {
            this->update(EPOLL_CTL_DEL, channel);
            channel->setIndex(deletedChannel);
        }
        else
            this->update(EPOLL_CTL_MOD, channel);
    }
}

void mg::Epoll::removeChannel(Channel *channel)
{
    if (!channel)
        return;
    this->_channels.erase(channel->fd());
    if (channel->index() == addedChannel)
        this->update(EPOLL_CTL_DEL, channel);
    channel->setIndex(deletedChannel);
}

void mg::Epoll::fillActiveChannels(int nums, std::vector<Channel *> &list)
{
    for (int i = 0; i < nums; i++)
    {
        Channel *channel = static_cast<Channel *>(this->_events[i].data.ptr);
        channel->setActiveEvents(this->_events[i].events);
        list.push_back(channel);
    }
}

void mg::Epoll::update(int operation, Channel *channel)
{
    struct epoll_event event;
    ::memset(&event, 0, sizeof(event));

    int fd = channel->fd();
    event.events = channel->events();
    event.data.ptr = static_cast<void *>(channel);

    if (::epoll_ctl(this->_epoll_fd, operation, fd, &event) == -1)
    {
        switch (operation)
        {
        case EPOLL_CTL_ADD:
            LOG_ERROR("epoll[{}] add error", this->_epoll_fd);
            break;
        case EPOLL_CTL_DEL:
            LOG_ERROR("epoll[{}] delete error", this->_epoll_fd);
            break;
        case EPOLL_CTL_MOD:
            LOG_ERROR("epoll[{}] modify error", this->_epoll_fd);
            break;
        default:
            break;
        }
    }
}
