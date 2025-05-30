#ifndef __SESSION_H__
#define __SESSION_H__

#include "../src/base/singleton.h"
#include "../src/protocol/IM.BaseDefine.pb.h"
#include "../src/protocol/IM.Message.pb.h"
#include "../src/protocol/IM.DataStruct.pb.h"

#include <cstdint>

class Session : public Singleton<Session>
{
public:
    uint32_t getSession(uint32_t from, uint32_t to, uint32_t type, bool tomb = false);

    uint32_t addSession(uint32_t from, uint32_t to, uint32_t type);

    uint32_t getRelation(uint32_t from, uint32_t to, bool insert = false);

    uint32_t addRelation(uint32_t from, uint32_t to);

    void saveMessage(uint32_t relation, IM::Message::MessageData &message);

    bool getMessage(std::vector<IM::DataStruct::UnReadMessage> &message);
};

#endif // __SESSION_H__