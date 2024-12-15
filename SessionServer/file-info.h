#ifndef __FILE_INFO_H__
#define __FILE_INFO_H__

#include <unordered_map>
#include <string>
#include <fstream>
#include <memory>

class FileInfo
{
public:
    enum FILEMODE
    {
        READ = 1,  // 读
        WRITE = 2, // 写
    };

    FileInfo(const std::string &name, const std::string &hash, uint32_t size, FILEMODE mode);

    ~FileInfo();

    static bool exist(std::string &name);

    inline const std::string &getName() { return this->_name; };

    /**
     * @param 设置文件分块大小
     */
    void setChunkSize(uint16_t size);

    /**
     * @brief 写入文件
     *
     * @param chunkIndex 写入第几个文件分块
     * @param data 写入数据
     * @return 本次写入文件数据字节
     */
    int32_t write(int16_t chunkIndex, const std::string &data);

private:
    int _fd;                                             // 写入文件的文件描述符
    std::string _name;                                   // 文件名（用户文件名）
    std::string _fileHash;                               // 文件hash值
    uint16_t _nums;                                      // 文件分块数
    uint16_t _chunkPerSize;                              // 分块文件大小
    uint32_t _size;                                      // 文件总大小
    std::unordered_map<int16_t, std::string> _chunkHash; // 分块文件的Hash值
    std::unordered_map<int16_t, uint32_t> _chunkSize;    // 分块文件目前的大小
};

#endif //__FILE_INFO_H__