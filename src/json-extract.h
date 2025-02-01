#ifndef __MG_JSON_EXTRACT_H__
#define __MG_JSON_EXTRACT_H__

#include "json.hpp"
#include <string>
#include <vector>

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

        inline static std::pair<bool, nlohmann::json> parse(const std::string &data)
        {
            nlohmann::json js;
            try
            {
                js = nlohmann::json::parse(data);
            }
            catch (const nlohmann::json::parse_error &e)
            {
                return std::make_pair(false, js);
            }
            return std::make_pair(true, js);
        }

        inline static bool parse(nlohmann::json &js, const std::string &data)
        {
            auto ret = parse(data);
            if (ret.first)
                js = std::move(ret.second);
            return ret.first;
        }
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
            output = it->get<T>();
            break;
        }
        case JsonType::STRING:
        {
            if (!it->is_string())
                return false;
            output = it->get<T>();
            break;
        }
        case JsonType::BINARY:
        {
            if (!it->is_binary())
                return false;
            output = it->get<T>();
            break;
        }
        case JsonType::OBJECT:
        {
            if (!it->is_object())
                return false;
            output = it->get<T>();
            break;
        }
        case JsonType::ARRAY:
        {
            if (!it->is_array())
                return false;
            output = it->get<T>();
            break;
        }
        default:
            return false;
        }
        return true;
    }
}

#endif //__MG_JSON_EXTRACT_H__
