#ifndef __MG_ACCEPTOR_H__
#define __MG_ACCEPTOR_H__

#include <functional>

#include "inet-address.h"
#include "socket.h"
#include "channel.h"
#include "noncopyable.h"

namespace mg
{
    class EventLoop;
    class Acceptor
    {
    public:
        using NewConnectionCallBack = std::function<void(int, const InternetAddress &)>;

        Acceptor(int domain, int type, EventLoop *loop, const InternetAddress &listenAddress, bool reusePort);

        ~Acceptor();

        bool isListening();

        void listen();

        void setNewConnectionCallBack(const NewConnectionCallBack callback);

    private:
        /**
         * @brief 仅用作创建非阻塞套接口
         *
         * @param domain DOMAIN_TYPE
         * @param type SOCKER_TYPE
         * @param return 返回创建的socket文件描述符句柄
         */
        int createNonBlockScoket(int domain, int type);

        /**
         * @brief 接受连接的套接口有读事件时的表现
         */
        void handleReadEvent();

        EventLoop *_loop;                // 属于哪一个EventLoop
        Socket _socket;                  // 用于接受新连接的socket
        Channel _channel;                // 用于_socket上发生的事件
        bool _listen;                    // 是否处于监听中
        NewConnectionCallBack _callback; // 新连接到来时的回调函数
        int _vacantFd;                   // 占用一个文件描述符，放置进程fd分配完毕后当新连接到来时无法accept
    };
};

#endif //__MG_ACCEPTOR_H__