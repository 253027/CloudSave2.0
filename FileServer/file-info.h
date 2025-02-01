#ifndef __FILE_INFO_H__
#define __FILE_INFO_H__

#include "md5.h"
#include "../src/rwlock.h"
#include "../src/macros.h"

#include <unordered_map>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

namespace mg
{
    class EventLoop;
};

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
        WAITING_INFO = 0, // 等待文件信息
        UPLOADING = 1,    // 上传中
        COMPLETED = 2,    // 上传完成
    };

    FileInfo(const std::string &name, FILEMODE mode, const std::string &dir = "./");

    FileInfo(const std::string &name, const std::string &hash, uint32_t size, FILEMODE mode, const std::string &dir = "./");

    ~FileInfo();

    static bool exist(std::string &name);

    inline const std::string &getName() { return this->_name; };

    /**
     * @brief 设置文件名
     */
    void setFileName(const std::string &filename);

    /**
     * @param 设置文件分块大小
     */
    void setChunkSize(uint32_t size);

    /**
     * @param 获取文件分块大小
     */
    const uint32_t getChunkSize() const;

    /**
     * @brief 获取文件分块数
     */
    const uint32_t getChunkNums() const;

    /**
     * @brief 设置文件hash值
     */
    void setFileHash(const std::string &hash);

    /**
     * @brief 设置文件大小
     */
    void setFileSize(uint32_t size);

    /**
     * @brief 得到文件名
     */
    const std::string &fileName() const;

    /**
     * @brief 写入文件
     *
     * @param chunkIndex 写入第几个文件分块
     * @param data 写入数据
     * @return 本次写入文件数据字节
     */
    int32_t write(int16_t chunkIndex, const std::string &data);

    /**
     * @brief 按块编号顺序写入
     * @return
     */
    int32_t sequenceWrite(int16_t chunkIndex, const std::string &data);

    /**
     * @brief 按块读取文件内容
     */
    std::string read(int16_t chunkIndex);

    /**
     * @brief 按字节范围读取文件内容
     */
    std::string read(uint32_t start, uint32_t nums);

    /**
     * @brief 设置文件状态
     */
    void setFileStatus(FILESTATUS status);

    /**
     * @brief 获取文件状态
     */
    FILESTATUS getFileStatus() const;

    /**
     * @brief 文件是数据否上传完成（未校验）
     */
    bool isCompleted();

    /**
     * @brief 文件数据大小是否完整（校验文件）
     */
    bool verify(bool needCalc = true);

    /**
     * @brief 动态计算MD5
     */
    bool update(std::vector<unsigned char> data);

    /**
     * @brief 设置所属计算线程
     */
    void setOwnerLoop(mg::EventLoop *loop);

    /**
     * @brief 得到所属计算线程
     */
    mg::EventLoop *getOwnerLoop();

    /**
     * @brief 得到文件大小
     */
    uint32_t getFileSize();

    /**
     * @brief 得到文件哈希值
     */
    const std::string &getFileHash() const;

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
    FileMD5 _md5;                                        // 文件校验
    uint32_t _calcIndex;                                 // 计算MD5的分块下标
    mg::EventLoop *_loop;                                // 所属eventloop
};

extern thread_local std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<FileInfo>>> fileInfoMemo;
#endif //__FILE_INFO_H__