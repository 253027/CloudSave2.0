#include "socket.h"
#include "log.h"

mg::Socket::Socket(int socket_fd)
    : socket_fd(socket_fd), type(0),
      domain(0)
{
    ;
}

mg::Socket::~Socket()
{
    ::close(this->socket_fd);
}

bool mg::Socket::setSocketType(int domain, int type)
{
    this->type = type;
    this->domain = domain;

    switch (domain)
    {
    case IPV4_DOMAIN:
        domain = AF_INET;
        break;
    case IPV6_DOMAIN:
        domain = AF_INET6;
        break;
    default:
        LOG_ERROR("Unknown domain: {}", domain);
        return false;
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
        LOG_ERROR("Unknown socket type: {}", type);
        return false;
    }

    socket_fd = ::socket(domain, type, 0);
    if (socket_fd == -1)
        LOG_ERROR("socket error");
    else
        LOG_DEBUG("New scoket fd[{}]", socket_fd);

    return socket_fd != -1;
}

bool mg::Socket::bind(const InternetAddress &address)
{
    int ret = 0;
    if (address._ipv6)
        ret = ::bind(this->socket_fd, (struct sockaddr *)&address._address._address6, sizeof(address._address._address6));
    else
        ret = ::bind(this->socket_fd, (struct sockaddr *)&address._address._address4, sizeof(address._address._address4));

    if (ret == -1)
    {
        LOG_ERROR("[{}]: {}", this->socket_fd, strerror(errno));
        return false;
    }
    return true;
}

bool mg::Socket::listen()
{
    if (::listen(this->socket_fd, SOMAXCONN) == -1)
    {
        LOG_ERROR("[{}]: {}", this->socket_fd, strerror(errno));
        return false;
    }
    return true;
}

int mg::Socket::accept(InternetAddress *peer_address)
{
    int connnect_fd = -1;
    socklen_t len;
    if (this->domain == IPV4_DOMAIN)
    {
        sockaddr_in address;
        len = sizeof(address);
        TEMP_FAILURE_RETRY(connnect_fd = ::accept4(this->socket_fd, (struct sockaddr *)&address, &len, SOCK_NONBLOCK | SOCK_CLOEXEC));
        if (peer_address)
            peer_address->_address._address4 = address;
    }
    else
    {
        sockaddr_in6 address;
        len = sizeof(address);
        TEMP_FAILURE_RETRY(connnect_fd = ::accept4(this->socket_fd, (struct sockaddr *)&address, &len, SOCK_NONBLOCK | SOCK_CLOEXEC));
        if (peer_address)
        {
            peer_address->_address._address6 = address;
            peer_address->_ipv6 = true;
        }
    }

    if (connnect_fd == -1)
        LOG_ERROR("[{}]: {}", this->socket_fd, strerror(errno));

    return connnect_fd;
}

void mg::Socket::setTcpNoDelay(bool on)
{
    int option = on ? 1 : 0;
    ::setsockopt(this->socket_fd, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(option));
}

void mg::Socket::setReuseAddress(bool on)
{
    int option = on ? 1 : 0;
    ::setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
}

void mg::Socket::setReusePort(bool on)
{
    int option = on;
    ::setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option));
}

void mg::Socket::setKeepLive(bool on)
{
    int option = on;
    ::setsockopt(this->socket_fd, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(option));
}

void mg::Socket::shutDownWrite()
{
    if (::shutdown(this->socket_fd, SHUT_WR) < 0)
        LOG_ERROR("[{}] {}", this->socket_fd, ::strerror(errno));
}

void mg::Socket::reset()
{
    ::close(this->socket_fd);
    this->socket_fd = 0;
}

mg::InternetAddress mg::Socket::getLocalAddress(int sockfd, bool isIpv6)
{
    InternetAddress address;
    if (isIpv6)
    {
        sockaddr_in6 &address6 = address._address._address6;
        ::memset(&address6, 0, sizeof(address6));
        socklen_t len = sizeof(address6);
        if (::getsockname(sockfd, (struct sockaddr *)&address6, &len) < 0)
            LOG_ERROR("socket[{}] get local_6 failed");
    }
    else
    {
        sockaddr_in6 &address4 = address._address._address6;
        ::memset(&address4, 0, sizeof(address4));
        socklen_t len = sizeof(address4);
        if (::getsockname(sockfd, (struct sockaddr *)&address4, &len) < 0)
            LOG_ERROR("socket[{}] get local_4 failed");
    }
    return std::move(address);
}

mg::InternetAddress mg::Socket::getPeerAddress(int sockfd, bool isIpv6)
{
    InternetAddress address;
    if (isIpv6)
    {
        sockaddr_in6 &address6 = address._address._address6;
        ::memset(&address6, 0, sizeof(address6));
        socklen_t len = sizeof(address6);
        if (::getpeername(sockfd, (struct sockaddr *)&address6, &len) < 0)
            LOG_ERROR("socket[{}] get peer_6 failed");
    }
    else
    {
        sockaddr_in6 &address4 = address._address._address6;
        ::memset(&address4, 0, sizeof(address4));
        socklen_t len = sizeof(address4);
        if (::getpeername(sockfd, (struct sockaddr *)&address4, &len) < 0)
            LOG_ERROR("socket[{}] get peer_4 failed");
    }
    return std::move(address);
}
