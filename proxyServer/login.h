#ifndef __LOGIN_H__
#define __LOGIN_H__

#include "../src/base/singleton.h"
#include "../src/common/common.h"

#include <string>

class Login : public Singleton<Login>
{
public:
    bool doLogin(const std::string &userName, const std::string &password, IM::DataStruct::UserInformation &info);
};

#endif // __LOGIN_H__