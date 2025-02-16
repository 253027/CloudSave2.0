#ifndef __CLIENT_CONNECTION_H__
#define __CLIENT_CONNECTION_H__

#include "../src/common/common.h"
#include "../src/base/tcp-connection.h"
#include "../src/protocal/imPduBase.h"

#include <string>

class ClientConnection : public ConnectionBase, public mg::TcpConnection
{
public:
    ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                     const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

    inline const std::string &getLoginName() const { return this->_loginName; }

    inline uint32_t getUserId() { return this->_userId; }

    inline void setUserId(uint32_t id) { this->_userId = id; }

private:
    void handleLoginRequest(PduMessage *message);

private:
    std::string _loginName; // 登录名
    uint32_t _userId;       // 登录ID
    uint32_t _clientType;   // 登录平台
};

#endif //__CLIENT_CONNECTION_H__