#ifndef __MESSAGE_SERVER_H__
#define __MESSAGE_SERVER_H__

#include "../src/base/singleton.h"

#include <vector>
#include <memory>

namespace mg
{
    class EventLoop;
    class TcpServer;
};

class LoginServerClient;

class MessageServer : public Singleton<MessageServer>
{
public:
    MessageServer();

    bool initial(const std::string &configPath);

    bool start();

    void quit();

    std::string getIp();

    uint16_t getPort();

    uint16_t getMaxConnection();

    bool loginServerAvaiable();

private:
    std::shared_ptr<mg::EventLoop> _loop;
    std::unique_ptr<mg::TcpServer> _server;
    std::vector<std::unique_ptr<LoginServerClient>> _loginClientList;
    uint16_t _maxConnection;
};

#endif //__MESSAGE_SERVER_H__