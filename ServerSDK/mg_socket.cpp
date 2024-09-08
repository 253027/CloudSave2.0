#include "mg_socket.h"

mg::Socket::Socket() : socket_fd(0)
{
    ;
}

mg::Socket::~Socket()
{
    ;
}

bool mg::Socket::setSocketType(int domain, int type)
{
    switch (domain)
    {
    case IPV4_DOMAIN:
        domain = AF_INET;
        break;
    case IPV6_DOMAIN:
        domain = AF_INET6;
        break;
    default:
        break;
    }

    switch (type)
    {
    case TCP_SOCKET:
        type = SOCK_STREAM;
        break;
    case UDP_SOCKET:
        type = SOCK_DGRAM;
        break;
    default:
        break;
    }

    socket_fd = ::socket(domain, type, 0);
    return socket_fd != -1;
}