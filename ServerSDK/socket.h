#ifndef __MG_SOCKET_H__
#define __MG_SOCKET_H__

#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "noncopyable.h"
#include "inet-address.h"

namespace mg
{

    enum SOCKER_TYPE
    {
        TCP_SOCKET = 1, // TCP套接字连接
        UDP_SOCKET = 2  // UDP套接字连接
    };

    enum DOMAIN_TYPE
    {
        IPV4_DOMAIN = 1, // IPV4地址
        IPV6_DOMAIN = 2  // IPV6地址
    };
};

namespace mg
{
    class Socket : noncopyable
    {
    public:
        Socket() : socket_fd(0) {}

        explicit Socket(int socket_fd);

        ~Socket();

        /**
         * @brief 返回socket对象管理的文件描述符
         */
        inline int fd() const { return socket_fd; }

        /**
         * @brief 设置套接口属性
         *
         * @param type 套接口SOCKER_TYPE枚举值
         * @return true 设置成功 false 设置失败
         */
        bool setSocketType(int domain, int type);

        /**
         * @brief 将封装过的地址绑定套套接口下
         */
        bool bind(const InternetAddress &address);

        /**
         * @brief 套接口实施监听
         */
        bool listen();

        /**
         *@brief 函数调用成功后，返回一个非负数表示该链接的文件描述，
         *       默认设置为非阻塞套接字，并将远端地址填入peer_address中
         *       如果发生错误，则返回-1，并且*peer_address被设置为不可使用的
         *
         * @return 返回接收到的连接代表的文件描述符
         */
        int accept(InternetAddress *peer_address);

        /**
         * @brief 是否禁用Nagle算法，对实时性有较高要求的启用
         */
        void setTcpNoDelay(bool on);

        /**
         * @brief 设置地址复用也就是处于Time-wait的地址
         */
        void setReuseAddress(bool on);

        /**
         * @brief 设置端口复用
         */
        void setReusePort(bool on);

        /**
         * @brief 设置长连接
         */
        void setKeepLive(bool on);

        /**
         * @brief 套接口关闭写端
         */
        void shutDownWrite();

        /**
         * @brief 重置socket
         */
        void reset();

        /**
         * @brief 根据套接口得到本地地址
         */
        static InternetAddress getLocalAddress(int sockfd, bool isIpv6 = false);

        /**
         * @brief 根据套接口得到远端地址
         */
        static InternetAddress getPeerAddress(int sockfd, bool isIpv6 = false);

    private:
        int socket_fd;

        int type;

        int domain;
    };
};

#endif //__MG_SOCKET_H__