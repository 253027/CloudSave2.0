#ifndef __BUSINESS_TYPE_H__
#define __BUSINESS_TYPE_H__

#include "../src/tcp-connection.h"
#include "../src/singleton.h"
#include "../src/json_fwd.hpp"
#include "../src/log.h"
#include "../protocal/protocal-session.h"

#include <unordered_map>
#include <memory>
#include <string>

class BusinessTask : public Singleton<BusinessTask>
{
public:
    bool parse(const mg::TcpConnectionPointer &connection, Protocal::SessionCommand &data);

private:
    using json = nlohmann::json;
    using TCPCONNECTION = const mg::TcpConnectionPointer;

    /**
     * @brief 登录模块
     */
    bool login(TCPCONNECTION &con, const json &jsData);

    /**
     * @brief 注册模块
     */
    bool regist(TCPCONNECTION &con, const json &jsData);
};

#endif //__BUSINESS_TYPE_H__