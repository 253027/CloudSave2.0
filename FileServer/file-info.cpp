#include "file-info.h"
#include "md5.h"
#include "../src/log.h"

#include <unistd.h>
#include <fcntl.h>
#include <numeric>

thread_local std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<FileInfo>>> fileInfoMemo; // 管理所有文件对象的集合
static const uint32_t initialChunkSize = 8192 * 1024;                                                                  // 8M bytes

FileInfo::FileInfo(const std::string &name, FILEMODE mode)
    : FileInfo(name, "", 0, mode)
{
    ;
}

FileInfo::FileInfo(const std::string &name, const std::string &hash, uint32_t size, FILEMODE mode)
    : _fd(-1), _status(0), _name(name), _fileHash(hash), _chunkPerSize(initialChunkSize), _size(size),
      _md5(), _calcIndex(0)
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
    if (data.size() > this->_chunkPerSize)
        return 0;

    int ret = ::pwrite(this->_fd, data.data(), data.size(), this->_chunkPerSize * std::max(0, static_cast<int>(chunkIndex)));
    LOG_DEBUG("{} index: {} wirte: {}", this->_name, chunkIndex, ret);
    return _chunkSize[chunkIndex] = ret;
}

bool FileInfo::exist(std::string &name)
{
    return ::access(name.c_str(), F_OK) == 0; // 后面可以改成查数据库获得
}

void FileInfo::setChunkSize(uint32_t size)
{
    this->_chunkPerSize = size;
    this->_nums = (this->_size + size - 1) / size;
}

void FileInfo::setFileHash(const std::string &hash)
{
    this->_fileHash = hash;
}

void FileInfo::setFileSize(uint32_t size)
{
    this->_size = size;
    this->_nums = (this->_size + _chunkPerSize - 1) / _chunkPerSize;
}

void FileInfo::setFileStatus(FILESTATUS status)
{
    this->_status = TO_UNDERLYING(status);
}

FileInfo::FILESTATUS FileInfo::getFileStatus() const
{
    return TO_ENUM(FILESTATUS, this->_status);
}

bool FileInfo::isCompleted()
{
    uint32_t size = std::accumulate(this->_chunkSize.begin(), this->_chunkSize.end(), 0,
                                    [](int value, const std::pair<const int, int> &t)
                                    {
                                        return value + t.second; //
                                    });
    return this->_nums == this->_chunkSize.size() && size == this->_size;
}

bool FileInfo::verify(bool needCalc)
{
    if (needCalc)
        return this->_fileHash == _md5.compute(this->fileName());
    return this->_fileHash == _md5.generate();
}

bool FileInfo::update(std::vector<unsigned char> data)
{
    if (!this->_chunkSize.count(this->_calcIndex))
        return false;

    this->_calcIndex++;
    this->_md5.update(data.data(), data.size());
    return true;
}

const uint32_t FileInfo::getChunkSize() const
{
    return this->_chunkPerSize;
}

const std::string &FileInfo::fileName() const
{
    return this->_name;
}

int32_t FileInfo::sequenceWrite(int16_t chunkIndex, const std::string &data)
{
    if (this->update(std::vector<unsigned char>(data.begin(), data.end())))
        return 0;
    return this->write(chunkIndex, data);
}

std::string FileInfo::read(int16_t chunkIndex)
{
    return std::string();
}