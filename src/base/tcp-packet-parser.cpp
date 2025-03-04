#include "tcp-packet-parser.h"
#include "tcp-connection.h"
#include "log.h"

bool mg::TcpPacketParser::send(const mg::TcpConnectionPointer con, const std::string &data)
{
    mg::Buffer buf;
    buf.appendInt32(data.size());
    buf.append(data);
    LOG_DEBUG("{} send: {} bytes", con->name(), buf.readableBytes());
    con->send(buf);
    return true;
}

bool mg::TcpPacketParser::reveive(const mg::TcpConnectionPointer con, std::string &data)
{
    if (con->_readBuffer.readableBytes() < headSize)
        return false;

    int len = con->_readBuffer.peekInt32();
    if (len < 0 || con->_readBuffer.readableBytes() < headSize + len) // FIXME: check the data maxsize
        return false;

    if (len != con->_readBuffer.readInt32())
    {
        LOG_ERROR("data size not compatiable");
        return false;
    }

    LOG_DEBUG("{} receive: {} bytes", con->name(), len);
    data = con->_readBuffer.retrieveAsString(len);
    return true;
}
