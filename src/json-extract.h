#ifndef __MG_JSON_EXTRACT_H__
#define __MG_JSON_EXTRACT_H__

#include "json.hpp"
#include <string>

namespace mg
{
    class JsonExtract
    {
    public:
        enum JsonType
        {
            INT,
            STRING,
            BINARY,
            OBJECT,
            ARRAY,
        };

        template <typename T>
        static bool extract(const nlohmann::json &js, const std::string &name, T &output, JsonType type);
    };

    template <typename T>
    inline bool JsonExtract::extract(const nlohmann::json &js, const std::string &name, T &output, JsonType type)
    {
        auto it = js.find(name);
        if (it == js.end())
            return false;

        switch (type)
        {
        case JsonType::INT:
        {
            if (!it->is_number_integer())
                return false;
            output = it->get<int>();
            break;
        }
        case JsonType::STRING:
        {
            if (!it->is_string())
                return false;
            output = it->get<std::string>();
            break;
        }
        }
        return true;
    }
}

#endif //__MG_JSON_EXTRACT_H__
