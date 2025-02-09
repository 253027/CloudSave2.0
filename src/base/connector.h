#ifndef __MG_CONNECTOR_H__
#define __MG_CONNECTOR_H__

#include "noncopyable.h"
#include "inet-address.h"
#include "function-callbacks.h"
#include "socket.h"

#include <memory>
#include <functional>

namespace mg
{
    class EventLoop;
    class Channel;
    class Connector : noncopyable, public std::enable_shared_from_this<Connector>
    {
    public:
        Connector(int domain, int type, EventLoop *loop, const InternetAddress &address);

        void setNewConnectionCallback(const std::function<void(int sockfd)> &callback);

        void start();

        void stop();

        void restart();

        const InternetAddress &getAddress() const;

    private:
        enum State
        {
            DisConnected = 1,
            Connecting = 2,
            Connected = 3
        };

        static const int _maxRetryDelayMileSeconds;     // 断线重连的最大时间
        static const int _initialRetryDelayMileSeconds; // 初始连接再次尝试连接的最大时间

        void startInLoop();

        void stopInLoop();

        void connect();

        void connecting();

        void setState(State state);

        int removeAndResetChannel();

        void resetChannel();

        void handleWrite();

        void handleError();

        void retry();

        int createNonBlockScoket(int domain, int type);

        EventLoop *_loop;                  // 所属事件循环
        InternetAddress _address;          // 连接地址
        std::unique_ptr<Channel> _channel; // 所属的channel类
        bool _connect;                     // 是否连接
        State _state;                      // 连接状态
        std::function<void(int sockfd)> _callback;
        int _domain; // 创建sockfd时的类型
        int _type;   // 创建sockfd时的类型
        Socket _socket;
        int _retryMileSeconds;
    };
};

#endif //__MG_CONNECTOR_H__