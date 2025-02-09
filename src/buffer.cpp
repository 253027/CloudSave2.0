#include "buffer.h"

#include <string.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>

mg::Buffer::Buffer(int initialSize)
    : _buffer(_headSize + initialSize),
      _readIndex(_headSize), _writeIndex(_headSize)
{
    ;
}

int mg::Buffer::writeableBytes()
{
    return this->_buffer.size() - this->_writeIndex;
}

int64_t mg::Buffer::peekInt64()
{
#ifndef _DEBUG
    assert(readableBytes() >= sizeof(int64_t));
#endif
    int32_t len2 = 0, len1 = 0;
    ::memcpy(&len2, readPeek(), sizeof(len2));
    ::memcpy(&len1, readPeek() + sizeof(len2), sizeof(len1));
    int64_t ret = static_cast<int64_t>(::ntohl(len2)) << 32;
    return ret | ::ntohl(len1);
}

int64_t mg::Buffer::readInt64()
{
    int ret = peekInt64();
    this->retrieve(sizeof(int64_t));
    return ret;
}

void mg::Buffer::appendInt64(int64_t len)
{
    int32_t len1 = ::htonl(len & 0xffffffff);
    int32_t len2 = ::htonl(len >> 32);
    append((char *)&len2, sizeof(int32_t));
    append((char *)&len1, sizeof(int32_t));
}

void mg::Buffer::appendInt32(int32_t len)
{
    len = ::htonl(len);
    append((char *)&len, sizeof(uint32_t));
}

int32_t mg::Buffer::peekInt32()
{
#ifndef _DEBUG
    assert(readableBytes() >= sizeof(int32_t));
#endif
    int32_t ret = 0;
    ::memcpy(&ret, readPeek(), sizeof(ret));
    return ::ntohl(ret);
}

int32_t mg::Buffer::readInt32()
{
    int32_t ret = peekInt32();
    this->retrieve(sizeof(int32_t));
    return ret;
}

void mg::Buffer::appendUInt16(uint16_t data)
{
    data = ::htons(data);
    append((char *)&data, sizeof(data));
}

uint16_t mg::Buffer::peekUInt16()
{
    uint16_t ret = 0;
    ::memcpy(&ret, readPeek(), sizeof(ret));
    return ::ntohs(ret);
}

uint16_t mg::Buffer::readUInt16()
{
    uint16_t data = peekUInt16();
    this->retrieve(sizeof(data));
    return data;
}

int mg::Buffer::readableBytes()
{
    return this->_writeIndex - this->_readIndex;
}

int mg::Buffer::hasReadBytes()
{
    return this->_readIndex;
}

const char *mg::Buffer::readPeek() const
{
    return this->begin() + this->_readIndex;
}

char *mg::Buffer::writePeek()
{
    return this->begin() + this->_writeIndex;
}

const char *mg::Buffer::writePeek() const
{
    return this->begin() + this->_writeIndex;
}

char *mg::Buffer::readPeek()
{
    return this->begin() + this->_readIndex;
}

void mg::Buffer::retrieve(int len)
{
    if (len < readableBytes())
        this->_readIndex += len;
    else
        this->retrieveAll();
}

void mg::Buffer::retrieveAll()
{
    this->_readIndex = this->_headSize;
    this->_writeIndex = this->_headSize;
}

std::string mg::Buffer::retrieveAllAsString()
{
    return retrieveAsString(this->readableBytes());
}

std::string mg::Buffer::retrieveAsString(int len)
{
    assert(len <= this->readableBytes());
    std::string res(this->readPeek(), len);
    retrieve(len);
    return res;
}

void mg::Buffer::append(const char *data, int len)
{
    this->ensureWriteSpace(len);
    std::copy(data, data + len, this->writePeek());
    this->_writeIndex += len;
}

void mg::Buffer::append(const std::string &data)
{
    this->append(data.data(), data.size());
}

int mg::Buffer::send(int fd, int &saveError)
{
    int len = ::write(fd, this->readPeek(), this->readableBytes());
    if (len < 0)
        saveError = errno;
    return len;
}

int mg::Buffer::receive(int fd, int &saveError)
{
    char extraBuffer[65536] = {0};

    int writeSize = this->writeableBytes();

    struct iovec vec[2];
    vec[0].iov_base = this->writePeek();
    vec[0].iov_len = writeSize;
    vec[1].iov_base = extraBuffer;
    vec[1].iov_len = sizeof(extraBuffer);

    const int vecSize = (writeSize < sizeof(extraBuffer)) ? 2 : 1;
    const int len = ::readv(fd, vec, vecSize);
    if (len < 0)
        saveError = errno;
    else if (len <= writeSize)
        this->_writeIndex += len;
    else
    {
        this->_writeIndex = this->_buffer.size();
        this->append(extraBuffer, len - writeSize);
    }
    return len;
}

char *mg::Buffer::begin()
{
    return this->_buffer.data();
}

const char *mg::Buffer::begin() const
{
    return this->_buffer.data();
}

void mg::Buffer::ensureWriteSpace(int len)
{
    if (this->writeableBytes() >= len)
        return;
    this->allocate(len);
    assert(this->writeableBytes() >= len);
}

void mg::Buffer::allocate(int len)
{
    /*
        实际上等价于 this->writeableBytes() + this->hasReadBytes() - this->_headSize < len
        扩容后大小变为this->_writeIndex + len，实际上空间大小增加len - this->_writeIndex字节
    */
    if (this->writeableBytes() + this->hasReadBytes() < len + this->_headSize)
        this->_buffer.resize(this->_writeIndex + len);
    else
    {
        int size = this->readableBytes();
        std::copy(this->readPeek(), this->writePeek(), this->begin() + this->_headSize);
        this->_readIndex = this->_headSize;
        this->_writeIndex = size + this->_readIndex;
    }
}
