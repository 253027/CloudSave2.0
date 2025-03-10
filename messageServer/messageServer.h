#ifndef __MESSAGE_SERVER_H__
#define __MESSAGE_SERVER_H__

#include "loginServerClient.h"
#include "proxyServerClient.h"
#include "../src/base/singleton.h"
#include "../src/base/tcp-server.h"

#include <vector>
#include <memory>

class MessageServer : public Singleton<MessageServer>
{
public:
    MessageServer();

    ~MessageServer();

    bool initial(const std::string &configPath);

    bool start();

    void quit();

    std::string getIp();

    uint16_t getPort();

    uint16_t getMaxConnection();

    bool loginServerAvaiable();

private:
    void acceptorCallback(int fd, const mg::InternetAddress &peerAddress);

private:
    std::string _ip;
    uint16_t _port;
    std::shared_ptr<mg::EventLoop> _loop;
    std::unique_ptr<mg::Acceptor> _acceptor;
    std::vector<std::unique_ptr<LoginServerClient>> _loginClientList;
    std::vector<std::unique_ptr<ProxyServerClient>> _proxyServertList;
    uint16_t _maxConnection;
};

#endif //__MESSAGE_SERVER_H__