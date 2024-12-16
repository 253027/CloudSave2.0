#include "file-info.h"
#include "../src/log.h"

#include <unistd.h>
#include <fcntl.h>

static const uint32_t initialChunkSize = 8192 * 1024; // 8M bytes

FileInfo::FileInfo(const std::string &name, const std::string &hash, uint32_t size, FILEMODE mode)
    : _name(name), _fileHash(hash), _chunkPerSize(initialChunkSize), _size(size), _fd(-1)
{
    int flags = (mode == FILEMODE::READ) ? O_RDONLY : (O_WRONLY | O_CREAT);
    mode_t permissions = (mode == FILEMODE::WRITE) ? 0644 : 0;

    this->_fd = ::open(name.c_str(), flags, permissions);
    if (this->_fd == -1)
        LOG_ERROR("Failed to open file {}: {}", name, std::strerror(errno));

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
