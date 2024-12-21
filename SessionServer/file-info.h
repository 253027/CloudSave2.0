#ifndef __FILE_INFO_H__
#define __FILE_INFO_H__

#include "../src/macros.h"

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

    // 文件对象的状态，决定了数据到来时的处理方法
    enum class FILESTATUS : uint16_t
    {
        WAITING_INFO = 1, // 等待文件信息
        UPLOADING = 2,    // 上传中
        COMPLETED = 3,    // 上传完成
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

    /**
     * @brief 设置文件状态
     */
    void setFileStatus(FILESTATUS status);

    /**
     * @brief 获取文件状态
     */
    FILESTATUS getFileStatus() const;

private:
    int _fd;                                             // 写入文件的文件描述符
    uint8_t _status;                                     // 文件状态
    std::string _name;                                   // 文件名（用户文件名）
    std::string _fileHash;                               // 文件hash值
    uint16_t _nums;                                      // 文件分块数
    uint32_t _chunkPerSize;                              // 分块文件大小
    uint32_t _size;                                      // 文件总大小
    std::unordered_map<int16_t, std::string> _chunkHash; // 分块文件的Hash值
    std::unordered_map<int16_t, uint32_t> _chunkSize;    // 分块文件目前的大小
};

extern std::unordered_map<std::string, std::unique_ptr<FileInfo>> fileMemo;
#endif //__FILE_INFO_H__