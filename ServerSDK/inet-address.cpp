#include "inet-address.h"

mg::InternetAddress::InternetAddress(uint16_t port, bool isIpv6)
{
    if (isIpv6)
    {
        ::memset(&_address._address6, 0, sizeof(_address._address6));
        _address._address6.sin6_family = AF_INET6;
        _address._address6.sin6_addr = in6addr_any;
        _address._address6.sin6_port = ::htons(port);
        _ipv6 = true;
    }
    else
    {
        ::memset(&_address._address4, 0, sizeof(_address._address4));
        _address._address4.sin_family = AF_INET;
        _address._address4.sin_port = ::htons(port);
        _address._address4.sin_addr.s_addr = ::htonl(INADDR_ANY);
    }
}

mg::InternetAddress::InternetAddress(const sockaddr_in &address)
{
    _address._address4 = address;
}

mg::InternetAddress::InternetAddress(const sockaddr_in6 &address)
{
    _address._address6 = address;
}

mg::InternetAddress::InternetAddress(const std::string &ip, uint16_t port, bool isIpv6)
{
    if (isIpv6 || ip.find(":") != ip.npos)
    {
        ::memset(&_address._address6, 0, sizeof(_address._address6));
        _address._address6.sin6_port = ::htons(port);
        ::inet_pton(AF_INET6, ip.c_str(), &_address._address6.sin6_addr);
        _address._address6.sin6_family = AF_INET6;
        _ipv6 = true;
    }
    else
    {
        ::memset(&_address._address4, 0, sizeof(_address._address4));
        _address._address4.sin_addr.s_addr = ::inet_addr(ip.c_str());
        _address._address4.sin_port = ::htons(port);
        _address._address4.sin_family = AF_INET;
    }
}

std::string mg::InternetAddress::toIp() const
{
    char buf[64] = {0};
    if (_ipv6)
        ::inet_ntop(AF_INET6, &_address._address6.sin6_addr, buf, sizeof(buf) - 1);
    else
        ::inet_ntop(AF_INET, &_address._address4.sin_addr, buf, sizeof(buf) - 1);
    return buf;
}

std::string mg::InternetAddress::toIpPort() const
{
    char buf[64] = {0};
    if (_ipv6)
    {
        ::inet_ntop(AF_INET6, &_address._address6.sin6_addr, buf, sizeof(buf));
        int len = ::strlen(buf);
        uint16_t port = ::ntohs(_address._address6.sin6_port);
        snprintf(buf + len, sizeof(buf) - 1, ":%u", port);
    }
    else
    {
        ::inet_ntop(AF_INET, &_address._address4.sin_addr, buf, sizeof(buf));
        int len = ::strlen(buf);
        uint16_t port = ::ntohs(_address._address4.sin_port);
        snprintf(buf + len, sizeof(buf) - 1, ":%u", port);
    }
    return buf;
}

uint16_t mg::InternetAddress::port() const
{
    return ::ntohs(_ipv6 ? _address._address6.sin6_port : _address._address4.sin_port);
}
