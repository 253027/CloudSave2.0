#ifndef __MG_POLLER_H__
#define __MG_POLLER_H__

#include <map>
#include <sys/epoll.h>

namespace mg
{
    class EventLoop;
    class Channel;

    class Poller
    {
    public:
        Poller(EventLoop *loop);

        virtual ~Poller() = default;

        virtual void updateChannel(Channel *channel) = 0;

        virtual void removeChannel(Channel *channel) = 0;

        virtual bool hasChannel(Channel *channel) const;

    protected:
        using ChannelMap = std::map<int, Channel *>;
        ChannelMap _channels;

    private:
        EventLoop *_loop;
    };
};

#endif //__MG_EPOLL_H__