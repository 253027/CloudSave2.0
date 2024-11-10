/**
 * @brief 封装epoll
 *
 * @author mogaitesheng
 *
 * @date 2024-09-16
 */

#ifndef __MG_EPOLL_H__
#define __MG_EPOLL_H__

#include <sys/epoll.h>
#include <vector>

#include "time-stamp.h"
#include "poller.h"

namespace mg
{
    class Channel;
    class EventLoop;

    class Epoll : public Poller
    {
    public:
        explicit Epoll(EventLoop *loop);

        ~Epoll() override;

        /**
         * @brief 内部调用epoll_wait，并将本次返回的事件填充进channelList中
         *
         * @param timeout epoll_wait等待时间（单位毫秒）
         */
        TimeStamp poll(std::vector<Channel *> &channelList, int timeout = -1);

        void updateChannel(Channel *channel) override;

        void removeChannel(Channel *channel) override;

    private:
        /**
         * @brief 将epoll返回的活跃channel填充进list中
         */
        void fillActiveChannels(int nums, std::vector<Channel *> &list);

        /**
         * @brief 更新Channel类在epoll中的状态
         *
         * @param operation 操作类型，如EPOLL_CTL_ADD
         * @param channel 待添加的channel对象
         */
        void update(int operation, Channel *channel);

        //::epoll返回的文件描述符
        int _epoll_fd;
        // 要管理的事件
        std::vector<struct epoll_event> _events;
        // epoll管理的事件初始化大小
        const int _events_initial_size = 128;
    };
}

#endif //__MG_EPOLL_H__