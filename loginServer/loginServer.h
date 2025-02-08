#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__

#include "../src/singleton.h"
#include "../src/acceptor.h"
#include "../src/inet-address.h"

#include <string>
#include <memory>

class LoginServer : public Singleton<LoginServer>
{
public:
    bool initial(const std::string &configPath);

    bool start();

    void quit();

private:
    void acceptorCallback(int fd, const mg::InternetAddress &peerAddress, int state);

private:
    std::shared_ptr<mg::EventLoop> _loop;         //_loop循环
    std::unique_ptr<mg::Acceptor> _messageServer; // 消息服务器接收器
    std::unique_ptr<mg::Acceptor> _httpClient;    // HTTP客户端接收器
    std::unique_ptr<mg::Acceptor> _client;        // 客户端接收器
    bool _start = false;
};

#endif //__LOGIN_SERVER_H__