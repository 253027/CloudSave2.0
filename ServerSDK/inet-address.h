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
        friend class Socket;

    public:
        explicit InternetAddress(uint16_t port = 0, bool isIpv6 = false);

        explicit InternetAddress(const sockaddr_in &address);

        explicit InternetAddress(const sockaddr_in6 &address);

        InternetAddress(const std::string &ip, uint16_t port, bool isIpv6 = false);

        std::string toIp() const;

        std::string toIpPort() const;

        uint16_t port() const;

        sockaddr_in &getSockAddress_4() { return _address._address4; };

        sockaddr_in6 &getSockAddress_6() { return _address._address6; };

        inline bool isIpv6() const { return this->_ipv6; };

    private:
        union Address
        {
            sockaddr_in _address4;
            sockaddr_in6 _address6;
        };

        bool _ipv6 = false;

        Address _address;
    };
};

#endif //__MG_INETADDRESS_H__