#ifndef __COMMON_MARCO_H__
#define __COMMON_MARCO_H__

#define PARSE_PROTOBUF_MESSAGE(TYPE, MESSAGE_VAR, DATA)             \
    TYPE MESSAGE_VAR;                                               \
    if (!(MESSAGE_VAR).ParseFromString(DATA))                       \
    {                                                               \
        LOG_ERROR("{} parse protobuf message error", this->name()); \
        return;                                                     \
    }

#define PARSE_JSON_FILE(JSON_VAR, CONFIG_PATH)                           \
    using json = nlohmann::json;                                         \
    json JSON_VAR;                                                       \
    {                                                                    \
        std::ifstream file(CONFIG_PATH);                                 \
        if (!file.is_open())                                             \
        {                                                                \
            LOG_ERROR("{} open file failed", CONFIG_PATH);               \
            return false;                                                \
        }                                                                \
        try                                                              \
        {                                                                \
            file >> JSON_VAR;                                            \
        }                                                                \
        catch (const json::parse_error &e)                               \
        {                                                                \
            LOG_ERROR("{} json parse error: {}", CONFIG_PATH, e.what()); \
        }                                                                \
        file.close();                                                    \
    }

#endif //__COMMON_MARCO_H__