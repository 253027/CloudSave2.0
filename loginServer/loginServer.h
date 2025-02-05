#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__

#include "../src/singleton.h"
#include "../src/acceptor.h"

#include <string>

class LoginServer : public Singleton<LoginServer>
{
public:
    bool initial(const std::string &configPath);

    bool start();

    void quit();

private:
    ;
};

#endif //__LOGIN_SERVER_H__