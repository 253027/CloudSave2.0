#include "mg_connection.h"

mg::Connection::Connection(const std::string &ip, int port, int domain, int type)
{
    if (!this->socket.setSocketType(domain, type))
        assert("套接口创建失败");

    struct sockaddr_in address;
    ::memset(&address, 0, sizeof(address));
    address.sin_family = (domain == mg::DOMAIN_TYPE::IPV4_DOMAIN) ? PF_INET : PF_INET6;
    address.sin_addr.s_addr = ::inet_addr(ip.c_str());
    address.sin_port = ::htons(port);

    int ret = ::bind(this->socket.fd(), (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1 && "套接口绑定失败");

    ret = ::listen(this->socket.fd(), SOMAXCONN);
    assert(ret != -1 && "套接口监听失败");
}

mg::Connection::~Connection()
{
    ;
}
