#ifndef __JSON_DATA_PARSER_H__
#define __JSON_DATA_PARSER_H__

#include "../ServerSDK/singleton.h"

#include <string>

class JsonDataParser : public Singleton<JsonDataParser>
{
public:
    JsonDataParser();

    void parse(const std::string &name, std::string &data);
};

#endif //__JSON_DATA_PARSER_H__