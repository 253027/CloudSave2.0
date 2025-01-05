#ifndef __BUSINESS_H__
#define __BUSINESS_H__

#include "../src/singleton.h"
#include "../src/http-packet-parser.h"
#include "../src/json.hpp"

#include <string>
#include <mutex>

class Business : public Singleton<Business>
{
public:
    Business();

    bool main(const mg::HttpRequest &request);

    bool login(const mg::HttpRequest &request);

private:
    bool parse(nlohmann::json &js, const std::string &data);

private:
    std::mutex _mutexMain; // main函数锁
};

#endif // __BUSINESS_H__