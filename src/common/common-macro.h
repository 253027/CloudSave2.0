#ifndef __COMMON_MARCO_H__
#define __COMMON_MARCO_H__

#define PARSE_PROTOBUF_MESSAGE(TYPE, MESSAGE_VAR, DATA)             \
    TYPE MESSAGE_VAR;                                               \
    if (!(MESSAGE_VAR).ParseFromString(DATA))                       \
    {                                                               \
        LOG_ERROR("{} parse protobuf message error", this->name()); \
        return;                                                     \
    }

#endif //__COMMON_MARCO_H__