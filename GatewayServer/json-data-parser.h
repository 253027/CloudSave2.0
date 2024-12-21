#ifndef __JSON_DATA_PARSER_H__
#define __JSON_DATA_PARSER_H__

#include "../src/singleton.h"

#include <string>
#include <unordered_map>

class JsonDataParser : public Singleton<JsonDataParser>
{
public:
    JsonDataParser();

    bool parse(const std::string &name, std::string &data);

private:
    enum Method
    {
        LOGIN = 1,
        REGIST = 2,
        UPLOAD = 3,
    };

    std::unordered_map<std::string, int> _method =
        {
            {"login", Method::LOGIN},
            {"regist", Method::REGIST},
            {"upload", Method::UPLOAD},
            //
        };
};

#endif //__JSON_DATA_PARSER_H__