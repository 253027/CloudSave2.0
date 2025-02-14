#ifndef __MG_TCP_SERVER_H__
#define __MG_TCP_SERVER_H__

#include "eventloop-threadpool.h"
#include "acceptor.h"
#include "tcp-connection.h"
#include "inet-address.h"
#include "event-loop.h"
#include "tcp-connection.h"

#include <unordered_map>
#include <memory>

namespace mg
{
    class TcpServer
    {
    public:
        using ThreadInitialCallBack = std::function<void(EventLoop *)>;

        TcpServer(EventLoop *loop, const InternetAddress &listenAddress, const std::string &name, int domain = IPV4_DOMAIN, int type = TCP_SOCKET);

        ~TcpServer();

        /**
         * @brief 设置回调函数
         * @param callback
         */
        void setThreadInitialCallback(ThreadInitialCallBack callback);

        /**
         * @brief 设置线程的数量
         * @param nums 线程数
         */
        void setThreadNums(int nums);

        /**
         * @brief 设置新连接建立时的回调，传递过程：用户自定义函数->TcpServer->TcpConnection
         */
        void setConnectionCallback(const TcpConnectionCallback &callback);

        /**
         * @brief 设置数据到来时处理函数，传递过程：用户自定义函数->TcpServer->TcpConnection
         *        该函数也是用户自定义的业务入口函数
         */
        void setMessageCallback(const MessageDataCallback &callback);

        /**
         * @brief 设置数据发送完成的回调函数，传递过程：用户自定义函数->TcpServer->TcpConnection
         * @param callback
         */
        void setWriteCompleteCallback(const WriteCompleteCallback &callback);

        /**
         * @brief 开启服务器
         */
        void start();

        /**
         * @brief 返回服务器实例的名字
         */
        const std::string &getName() const;

        /**
         * @brief 得到服务器绑定的IP和端口号
         */
        std::string getIpPort();

        /**
         * @brief 得到本机监听IP
         */
        std::string getIp();

        /**
         * @brief 得到本机监听端口
         */
        uint16_t getPort();

    private:
        /**
         * @brief acceptor类需要的回调函数
         */
        void acceptorCallback(int fd, const InternetAddress &peerAddress);

        /**
         * @brief 移除连接
         */
        void removeConnection(const TcpConnectionPointer &connection);

        /**
         * @brief 移除连接时需要_loop执行的函数
         */
        void removeConnectionCallBack(const TcpConnectionPointer &connection);

        bool _isStarted;                                                                 // 是否启动
        std::string _name;                                                               // 服务器的名称
        EventLoop *_loop;                                                                // 用户定义的mainloop
        std::unique_ptr<Acceptor> _acceptor;                                             // 用于接受新连接的类实例
        int _connectionID;                                                               // 用于区分相同TCP连接时不同的名字
        ThreadInitialCallBack _threadInitialCallback;                                    // loop线程初始化的回调函数
        std::shared_ptr<EventLoopThreadPool> _threadPool;                                // 线程池
        InternetAddress _address;                                                        // 绑定的地址
        std::unordered_map<std::string, std::shared_ptr<TcpConnection>> _connectionMemo; // 管理所有连接

        /*-------以下是保存用户自定义的函数--------*/
        TcpConnectionCallback _connectionCallback;    // 新链接回调
        WriteCompleteCallback _writeCompleteCallback; // 写数据完成后回调
        MessageDataCallback _messgageDataCallback;    // 数据处理回调
    };
}

#endif //__MG_TCP_SERVER_H__