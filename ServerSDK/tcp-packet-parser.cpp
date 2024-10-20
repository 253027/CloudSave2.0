#include "tcp-packet-parser.h"
#include "tcp-connection.h"
#include "log.h"

bool mg::TcpPacketParser::send(const mg::TcpConnectionPointer con, std::string &data)
{
    int len = ::htonl(data.size());
    std::string buf((char *)&len, headSize);
    buf += data;
    con->send(buf);
    return true;
}

bool mg::TcpPacketParser::reveive(const mg::TcpConnectionPointer con, std::string &data)
{
    if (con->_readBuffer.readableBytes() < headSize)
        return false;

    int len = ::ntohl(*((int *)con->_readBuffer.readPeek()));
    if (con->_readBuffer.readableBytes() < headSize + len)
        return false;

    if (len != ::ntohl(*((int *)con->_readBuffer.retrieveAsString(headSize).c_str())))
    {
        LOG_ERROR("data size not compatiable");
        return false;
    }

    data = con->_readBuffer.retrieveAsString(len);
    return true;
}
