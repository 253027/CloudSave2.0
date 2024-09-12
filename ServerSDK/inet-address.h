#ifndef __MG_INETADDRESS_H__
#define __MG_INETADDRESS_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <string.h>

namespace mg
{
    class InternetAddress
    {
    public:
        explicit InternetAddress(uint16_t port = 0, bool isIpv6 = false);

        explicit InternetAddress(const sockaddr_in &address);

        explicit InternetAddress(const sockaddr_in6 &address);

        InternetAddress(const std::string &ip, uint16_t port, bool isIpv6 = false);

        std::string toIp() const;

        std::string toIpPort() const;

        uint16_t port() const;

    private:
        union
        {
            sockaddr_in _address4;
            sockaddr_in6 _address6;
        };

        bool _ipv6 = false;
    };
};

#endif //__MG_INETADDRESS_H__