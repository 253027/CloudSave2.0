#include "socket.h"
#include "inet-address.h"
#include "log.h"

mg::Socket::Socket(int socket_fd) : socket_fd(socket_fd) {}

mg::Socket::~Socket() {}

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

    this->type = type;
    this->domain = domain;

    return socket_fd != -1;
}

void mg::Socket::bind(const InternetAddress &address)
{
    ;
}

void mg::Socket::listen()
{
    ;
}

int mg::Socket::accept(InternetAddress *peer_address)
{
    int connnect_fd = -1;
    socklen_t len;
    if (this->domain == IPV4_DOMAIN)
    {
        sockaddr_in address;
        len = sizeof(address);
        connnect_fd = ::accept4(this->socket_fd, (sockaddr *)&address, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (peer_address)
            peer_address->_address4 = address;
    }
    else
    {
        sockaddr_in6 address;
        len = sizeof(address);
        connnect_fd = ::accept4(this->socket_fd, (sockaddr *)&address, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (peer_address)
        {
            peer_address->_address6 = address;
            peer_address->_ipv6 = true;
        }
    }

    if (connnect_fd == -1)
        LOG_DEBUG("socket:{} accept4() failed", this->socket_fd);

    return connnect_fd;
}