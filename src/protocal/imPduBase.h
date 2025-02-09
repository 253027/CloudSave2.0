#ifndef __IM_PDU_BASE_H__
#define __IM_PDU_BASE_H__

#include "../base/buffer.h"
#include "../../protobuf/include/google/protobuf/message_lite.h"

#include <cstdint>

struct PduHeader
{
    uint32_t length;
    uint16_t version;
    uint16_t flag;
    uint16_t service_id;
    uint16_t command_id;
    uint16_t sequence_number;
    uint16_t reversed;
};

class PduMessage
{
public:
    PduMessage();

    void setVersion(uint16_t version);

    void setFlag(uint16_t flag);

    void setServiceId(uint16_t id);

    void setCommandId(uint16_t id);

    void setSequenceNumber(uint16_t num);

    bool setPBMessage(const google::protobuf::MessageLite *message);

    std::string dump();

private:
    PduHeader _head;
    mg::Buffer _buffer;
};

#endif //__IM_PDU_BASE_H__