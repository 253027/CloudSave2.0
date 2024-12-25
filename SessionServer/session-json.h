#ifndef __SESSION_JSON_H__
#define __SESSION_JSON_H__

#include "../src/json.hpp"

#include <string>

class SessionJson
{
    using json = nlohmann::json;

public:
    inline static json generate(const std::string &name)
    {
        json js;
        js["connection-name"] = name;
        return js;
    }

    inline static json generate(json &js)
    {
        return generate(js["connection-name"]);
    }
};

#endif // __SESSION_JSON_H__