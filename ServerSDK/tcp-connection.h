#ifndef __MG_TCP_CONNECTION_H__
#define __MG_TCP_CONNECTION_H__

#include "socket.h"
#include "channel.h"
#include "inet-address.h"
#include "function-callbacks.h"
#include "noncopyable.h"
#include "buffer.h"

#include <memory>
#include <atomic>

namespace mg
{
    class EventLoop;
    class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
    {
    public:
        TcpConnection(EventLoop *loop, const std::string &name, int sockfd,
                      const InternetAddress &localAddress, const InternetAddress &peerAddress);

        ~TcpConnection();

        inline const std::string &name() const { return this->_name; }

        inline EventLoop *getLoop() { return this->_loop; }

        inline const InternetAddress &localAddress() const { return this->_localAddress; }

        inline const InternetAddress &peerAddress() const { return this->_peerAddress; }

        void setConnectionCallback(TcpConnectionCallback callback);

        void setMessageCallback(MessageDataCallback callback);

        void setWriteCompleteCallback(WriteCompleteCallback callback);

        void setCloseCallback(ConnectionClosedCallback callback);

        void setHighWaterMarkCallback(HighWaterMarkCallback callback, int len);

        bool connected();

        void shutdown();

        /**
         * @brief 发送数据
         * @param data 待发送的数据
         */
        void send(const std::string &data);
        void send(Buffer &data);

        /**
         * @brief TcpServer接受到新连接需要处理的逻辑，这里放在TcpConnection类中，
         *        因为一个连接的建立与销毁的操作只与该链接有关
         */
        void connectionEstablished();
        void connectionDestoryed();

        /**
         * @brief 设置用户自定义连接状态
         * @param state 状态值
         */
        void setUserConnectionState(int state);

        /**
         * @brief 得到用户自定义连接状态
         * @return 自定义连接状态
         */
        int getUserConnectionState();

        friend class TcpPacketParser;

    private:
        enum State
        {
            DISCONNECTED = 1, // 已经断开连接
            CONNECTING = 2,   // 正在连接
            CONNECTED = 3,    // 已连接
            DISCONNECTING = 4 // 断开连接
        };

        void setConnectionState(State state);

        /**
         * @brief 处理套接口可读事件
         */
        void handleRead(TimeStamp time);

        /**
         * @brief 套接口连接关闭事件
         */
        void handleClose();

        /**
         * @brief 处理连接错误事件
         */
        void handleError();

        /**
         * @brief 处理连接写事件
         */
        void handleWrite();

        /**
         * @brief 在所属的loop中关闭连接
         */
        void shutDownInOwnerLoop();

        /**
         * @brief 在自己所属的loop中发送数据
         * @param data 待发送的数据
         * @param len 待发送数据的长度
         */
        void sendInOwnerLoop(const void *data, int len);
        void sendInOwnerLoop(const std::string &data);

        int _highWaterMark;                           // 高水位阈值
        EventLoop *_loop;                             // 所属的事件循环
        std::string _name;                            // 连接名称
        std::unique_ptr<Socket> _socket;              // 使用的套接口
        std::unique_ptr<Channel> _channel;            // 管理套口事件的Channel类
        std::atomic<State> _state;                    // 连接状态
        const InternetAddress _localAddress;          // 本端IP地址
        const InternetAddress _peerAddress;           // 对端IP地址
        TcpConnectionCallback _connectionCallback;    // 连接建立或者断开时的回调
        MessageDataCallback _messageCallback;         // 有读写消息时的回调
        WriteCompleteCallback _writeCompleteCallback; // 消息发送完毕时的回调
        ConnectionClosedCallback _closeCallback;      // 连接关闭时的回调
        HighWaterMarkCallback _highWaterCallback;     // 写缓冲区数据过多执行的回调
        Buffer _sendBuffer;                           // 写缓冲区
        Buffer _readBuffer;                           // 读缓冲区
        std::atomic_int _userStat;                    // 用户自定义的Tcp连接状态
    };
};

#endif //__MG_TCP_CONNECTION_H__