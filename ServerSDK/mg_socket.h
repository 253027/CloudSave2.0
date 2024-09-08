#ifndef __MG_SOCKET_H__
#define __MG_SOCKET_H__

#include <unistd.h>
#include <sys/socket.h>

#include "mg_noncopyable.h"

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
        Socket();

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

    private:
        int socket_fd;
    };
};

#endif //__MG_SOCKET_H__