#ifndef __BUSINESS_H__
#define __BUSINESS_H__

#include "../src/singleton.h"
#include "../src/http-packet-parser.h"
#include "../src/json.hpp"

#include <string>

class Business : public Singleton<Business>
{
public:
    bool main(const mg::HttpRequest &request);

    bool login(const mg::HttpRequest &request);

private:
    bool parse(nlohmann::json &js, const std::string &data);
};

#endif // __BUSINESS_H__