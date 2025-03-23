#include "tcp-packet-parser.h"
#include "tcp-connection.h"
#include "log.h"

const int maxSize = 1024 * 1024;

bool mg::TcpPacketParser::send(const mg::TcpConnectionPointer con, const std::string &data)
{
    mg::Buffer buf;
    buf.appendInt32(data.size());
    buf.append(data);
    LOG_TRACE("{} send: {} bytes", con->name(), buf.readableBytes());
    con->send(buf);
    return true;
}

bool mg::TcpPacketParser::reveive(const mg::TcpConnectionPointer con, std::string &data)
{
    if (con->_readBuffer.readableBytes() < headSize)
        return false;

    int len = con->_readBuffer.peekInt32();
    if (len >= maxSize)
    {
        uint32_t retriveLen = con->_readBuffer.retrieveAllAsString().size();
        LOG_ERROR("{} too large data size {}, refuse accept and discard size {}", con->name(), len, retriveLen);
        return false;
    }

    if (len < 0 || con->_readBuffer.readableBytes() < headSize + len)
        return false;

    if (len != con->_readBuffer.readInt32())
    {
        LOG_ERROR("data size not compatiable");
        return false;
    }

    LOG_TRACE("{} receive: {} bytes", con->name(), len);
    data = con->_readBuffer.retrieveAsString(len);
    return true;
}
