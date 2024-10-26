#ifndef __BUSINESS_TYPE_H__
#define __BUSINESS_TYPE_H__

#include "../ServerSDK/tcp-connection.h"
#include "../ServerSDK/singleton.h"
#include "../ServerSDK/json_fwd.hpp"
#include "../ServerSDK/log.h"

#include <unordered_map>
#include <memory>
#include <string>

class BusinessTask : public Singleton<BusinessTask>
{
public:
    void parse(const mg::TcpConnectionPointer &connection, const std::string &data);

private:
    using json = nlohmann::json;
    using TCPCONNECTION = const mg::TcpConnectionPointer;

    enum METHODTYPE
    {
        LOGIN = 1, // 登录
    };

    /**
     * @brief 登录模块
     */
    void login(TCPCONNECTION &con, const json &jsData);
};

#endif //__BUSINESS_TYPE_H__