#include "file-info.h"
#include "../src/log.h"

#include <unistd.h>
#include <fcntl.h>

static const uint16_t initialChunkSize = 8192; // 8M bytes

FileInfo::FileInfo(const std::string &name, const std::string &hash, uint32_t size, FILEMODE mode)
    : _name(name), _fileHash(hash), _chunkPerSize(initialChunkSize), _size(size)
{
    switch (mode)
    {
    case FILEMODE::READ:
    {
        this->_fd = ::open(name.c_str(), O_RDONLY);
        break;
    }
    case FILEMODE::WRITE:
    {
        this->_fd = ::open(name.c_str(), O_WRONLY | O_CREAT);
        break;
    }
    }
    this->_nums = (this->_size + _chunkPerSize - 1) / _chunkPerSize;
}

FileInfo::~FileInfo()
{
    TEMP_FAILURE_RETRY(::close(this->_fd));
}

int32_t FileInfo::write(int16_t chunkIndex, const std::string &data)
{
    if (chunkIndex >= this->_nums)
    {
        LOG_DEBUG("{} invalid chunkindex maxsize:{} input-size:{}", this->_name, this->_nums, chunkIndex);
        return 0;
    }
    int ret = ::pwrite(this->_fd, data.data(), data.size(), this->_chunkPerSize * std::max(0, chunkIndex - 1));
    LOG_DEBUG("{} index: {} wirte: {}", this->_name, chunkIndex, ret);
    return ret;
}

bool FileInfo::exist(std::string &name)
{
    return ::access(name.c_str(), F_OK) == 0;
}

void FileInfo::setChunkSize(uint16_t size)
{
    this->_chunkPerSize = size;
    this->_nums = (this->_size + size - 1) / size;
}
