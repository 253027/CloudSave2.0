#ifndef __FILE_INFO_H__
#define __FILE_INFO_H__

#include <unordered_map>
#include <string>
#include <fstream>
#include <memory>

class FileInfo
{
public:
    FileInfo(const std::string &name, const std::string &hash, uint32_t size);

    ~FileInfo();

    inline const std::string &getName() { return this->_name; };

private:
    int _fd;                                             // 写入文件的文件描述符
    std::string _name;                                   // 文件名（用户文件名）
    std::string _fileHash;                               // 文件hash值
    uint16_t _nums;                                      // 文件分块数
    uint32_t _size;                                      // 文件总大小
    std::unordered_map<int16_t, std::string> _chunkHash; // 分块文件的Hash值
    std::unordered_map<int16_t, uint32_t> _chunkSize;    // 分块文件目前的大小
};

#endif //__FILE_INFO_H__