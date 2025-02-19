#ifndef __MG_TCP_CLIENT_H__
#define __MG_TCP_CLIENT_H__

#include "noncopyable.h"
#include "function-callbacks.h"
#include "inet-address.h"
#include "socket.h"

#include <mutex>

namespace mg
{
    class EventLoop;
    class TcpClient : public noncopyable
    {
    public:
        TcpClient(int domain, int type, EventLoop *loop, const InternetAddress &address, const std::string &name);

        ~TcpClient();

        /**
         * @brief 建立连接
         */
        void connect();

        /**
         * @brief 断开连接
         */
        void stop();

        /**
         * @brief 半关闭
         */
        void disconnect();

        /**
         * @brief 启用断线重连
         */
        void enableRetry();

        /**
         * @brief 取消断线重连
         */
        void disableRetry();

        /**
         * @brief 设置新连接建立时的回调，传递过程：用户自定义函数->TcpClient->TcpConnection
         */
        void setConnectionCallback(TcpConnectionCallback callback);

        /**
         * @brief 设置数据到来时处理函数，传递过程：用户自定义函数->TcpClient->TcpConnection
         *        该函数也是用户自定义的业务入口函数
         */
        void setMessageCallback(MessageDataCallback callback);

        /**
         * @brief 设置数据发送完成的回调函数，传递过程：用户自定义函数->TcpClient->TcpConnection
         */
        void setWriteCompleteCallback(WriteCompleteCallback callback);

        /**
         * @brief 返回链接是否可用
         */
        bool connected();

        /**
         * @brief 获得连接实例
         */
        TcpConnectionPointer connection();

    private:
        /**
         * @brief 新连接建立时执行的函数
         */
        void newConnection(int sockfd);

        /**
         * @brief 移除连接时的回调
         */
        void removeConntction(const TcpConnectionPointer &connection);

        EventLoop *_loop;                 // 所属时间循环
        const std::string _name;          // 线程名
        bool _connected;                  // 连接是否已建立
        std::mutex _mutex;                // 保护_connection连接实例
        TcpConnectionPointer _connection; // 连接实例
        ConnectorPointer _connector;      // 连接器实例
        bool _isIpv6;                     // 是否是IPV6地址
        int _connectionID;                // 标识每一条连接
        bool _retry;                      // 断开连接后是否重连

        /*-------以下是保存用户自定义的函数--------*/
        TcpConnectionCallback _connectionCallback;    // 新连接回调
        WriteCompleteCallback _writeCompleteCallback; // 写数据完成后回调
        MessageDataCallback _messgageDataCallback;    // 数据处理回调
    };
};

#endif //__MG_TCP_CLIENT_H__