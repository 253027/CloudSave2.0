#include "imPduBase.h"

#include <string.h>

PduMessage::PduMessage()
{
    ::memset(&this->_head, 0, sizeof(this->_head));
}

void PduMessage::setVersion(uint16_t version)
{
    this->_head.version = version;
}

void PduMessage::setFlag(uint16_t flag)
{
    this->_head.flag = flag;
}

void PduMessage::setServiceId(uint16_t id)
{
    this->_head.service_id = id;
}

void PduMessage::setCommandId(uint16_t id)
{
    this->_head.command_id = id;
}

void PduMessage::setSequenceNumber(uint16_t num)
{
    this->_head.sequence_number = num;
}

bool PduMessage::setPBMessage(const google::protobuf::MessageLite *message)
{
    std::vector<u_char> buffer(message->ByteSizeLong());
    if (!message->SerializeToArray(buffer.data(), message->ByteSizeLong()))
        return false;
    this->_buffer.append(reinterpret_cast<char *>(buffer.data()), buffer.size());
    return true;
}

bool PduMessage::parse(std::string &data)
{
    if (data.size() < sizeof(this->_head))
        return false;

    this->_buffer.append(std::move(data));
    this->_head.length = this->_buffer.readInt32();
    this->_head.version = this->_buffer.readUInt16();
    this->_head.flag = this->_buffer.readUInt16();
    this->_head.service_id = this->_buffer.readUInt16();
    this->_head.command_id = this->_buffer.readUInt16();
    this->_head.sequence_number = this->_buffer.readUInt16();
    this->_head.reversed = this->_buffer.readUInt16();

    return true;
}

uint16_t PduMessage::getVersion()
{
    return this->_head.version;
}

uint16_t PduMessage::getFlag()
{
    return this->_head.flag;
}

uint16_t PduMessage::getServiceId()
{
    return this->_head.service_id;
}

uint16_t PduMessage::getCommandId()
{
    return this->_head.command_id;
}

mg::Buffer PduMessage::getPBmessage()
{
    return this->_buffer;
}

std::string PduMessage::dump()
{
    mg::Buffer headBuffer;
    headBuffer.appendInt32(this->_head.length);
    headBuffer.appendUInt16(this->_head.version);
    headBuffer.appendUInt16(this->_head.flag);
    headBuffer.appendUInt16(this->_head.service_id);
    headBuffer.appendUInt16(this->_head.command_id);
    headBuffer.appendUInt16(this->_head.sequence_number);
    headBuffer.appendUInt16(this->_head.reversed);
    headBuffer.append(this->_buffer.retrieveAllAsString());
    return headBuffer.retrieveAllAsString();
}