#ifndef __MG_CHANNEL_H__
#define __MG_CHANNEL_H__

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "time-stamp.h"

namespace mg
{
    class EventLoop;
    const int newChannel = -1;
    const int deletedChannel = 2;
    const int addedChannel = 3;

    class Channel : noncopyable
    {
    public:
        // 事件循环回调函数
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(TimeStamp)>;

        /**
         * @brief Channel需要知道自己管理的那一个fd，以及自己属于那一个EventLoop
         *        因此构造函数必须带有loop和fd进行初始化
         */
        Channel(EventLoop *loop, int fd);

        ~Channel();

        /**
         * @brief 处理事件回调函数
         */
        void handleEvent(TimeStamp receiveTime);

        /**
         * @brief 设置读事件回调函数
         */
        void setReadCallback(ReadEventCallback readBack);

        /**
         * @brief 设置写事件回调函数
         */
        void setWriteCallback(EventCallback callBack);

        /**
         * @brief 设置套接口关闭回调函数
         */
        void setCloseCallback(EventCallback callBack);

        /**
         * @brief 设置套接口发生错误时的回调函数
         */
        void setErrorCallback(EventCallback callBack);

        /**
         * @brief 返回管理的文件描述符fd
         */
        int fd() const;

        /**
         * @brief 返回感兴趣的事件
         */
        int events() const;

        /**
         * @brief 向epoll注册写事件
         */
        void enableWriting();

        /**
         * @brief 关闭写事件
         */
        void disableWriting();

        /**
         * @brief 向epoll注册读事件
         */
        void enableReading();

        /**
         * @brief 关闭读事件
         */
        void disableReading();

        /**
         * @brief 设置poll返回时Channel类关注的已触发的事件集合
         */
        void setActiveEvents(int activeEvents);

        /**
         * @brief 设置在监听的集合中的状态
         */
        void setIndex(int index);

        /**
         * @brief 获取在监听的集合中的状态
         */
        int index() const;

        /**
         * @brief channel管理的对象是否没有感兴趣的事件
         */
        bool isNoneEvent() const;

        /**
         * @brief 是否注册了可写事件
         * @return true 是 false不是
         */
        bool isWriting() const;

        /**
         * @brief 是否注册了可读事件
         * @return true 是 false不是
         */
        bool isReading() const;

        /**
         * @brief 将channel所感兴趣的事件全部设置为0
         */
        void disableAllEvents();

        /**
         * @brief 将该channel从所属的_loop对象中移除
         */
        void remove();

        /**
         * @brief 将TcpConnection的共享指针和Channel成员弱指针绑定tie_，
         *        防止正在处理事务的时候TcpConnection已经被析构释放掉了
         */
        void tie(const std::shared_ptr<void> &);

        /**
         * @brief 返回channel所属的eventlopp类实例
         */
        EventLoop *ownerLoop();

    private:
        void update();

        void handleEventWithGuard(TimeStamp time);

        EventLoop *_loop;

        const int _fd;

        int _events;

        int _activeEvents;
        // 在监听的集合中的状态
        int _index;

        EventCallback _writeCallback;

        EventCallback _closeCallback;

        EventCallback _errorCallback;

        ReadEventCallback _readCallback;

        std::weak_ptr<void> _tie;

        bool _tied;

        bool _handleEvent;
    };
};

#endif //__MG_CHANNEL_H__