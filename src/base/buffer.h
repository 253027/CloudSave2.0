#ifndef __MG_BUFFER_H__
#define __MG_BUFFER_H__

#include <vector>
#include <string>
#include <algorithm>
#include <stddef.h>
#include <assert.h>
#include <cstdint>

namespace mg
{
    class Buffer
    {
    public:
        Buffer(int initialSize = _initialSize);

        /**
         * @brief 获取缓冲区中可读数据的起始地址
         */
        char *readPeek();
        const char *readPeek() const;

        /**
         * @brief 取出缓冲区中所有数据并返回数据
         */
        std::string retrieveAllAsString();

        /**
         * @brief 取出缓冲区中指定长度的数据
         * @param len 要去出的数据长度
         */
        std::string retrieveAsString(int len);

        /**
         * @brief 将数据写入缓冲区中
         * @param data 要写入的数据起始地址
         * @param len 写入数据的长度
         */
        void append(const char *data, int len);

        /**
         * @brief 将数据写入缓冲区中
         * @param data 数据
         */
        void append(const std::string &data);

        /**
         * @brief 发送数据
         * @param fd 要发送的socket文件描述符
         * @param saveError 保存发生错误时的状态
         */
        int send(int fd, int &saveError);

        /**
         * @brief 接收数据
         * @param fd 要接受的socket文件描述符
         * @param saveError 保存发生错误时的状态
         */
        int receive(int fd, int &saveError);

        /**
         * @brief 从可读区域中取出数据
         * @param len 取出数据的字节数
         */
        void retrieve(int len);

        /**
         * @brief 获取可以读数据的大小
         */
        int readableBytes();

        /**
         * @brief 获取缓存发送数据的大小
         */
        int writeableBytes();

        /**
         * @brief 从缓冲区读int64_t大小字节的数据, peekInt64()只读取不取出数据,
         *        readInt64()读取并取出
         */
        int64_t peekInt64();
        int64_t readInt64();

        /**
         * @brief 将sizeof(len)大小数据附加到缓冲区中
         */
        void appendInt64(int64_t len);
        void appendInt32(int32_t len);

        /**
         * @brief 从缓冲区读int32_t大小字节的数据, peekInt32()只读取不取出数据,
         *        readInt32()读取并取出
         */
        int32_t peekInt32();
        int32_t readInt32();

        /**
         * @brief 将sizeof(len)大小数据附加到缓冲区中
         */
        void appendUInt16(uint16_t data);
        uint16_t peekUInt16();
        uint16_t readUInt16();

        static const int _headSize = 4;       // 每个数据包的包头长度
        static const int _initialSize = 1024; // 缓冲区长度

    private:
        /**
         * @brief 获取已经读取的数据大小
         */
        int hasReadBytes();

        /**
         * @brief 获取_buffer的地址
         * @return 返回_buffer的首地址指针
         */
        char *begin();
        const char *begin() const;

        /**
         * @brief 获取缓冲区中可写数据的起始地址
         */
        char *writePeek();
        const char *writePeek() const;

        /**
         * @brief Buffer空间不足时至少分配至给定大小
         * @param len 需要的空间
         */
        void ensureWriteSpace(int len);

        /**
         * @brief 调整buffer空间至少有len大小
         */
        void allocate(int len);

        /**
         * @brief 取出所有可读数据
         */
        void retrieveAll();

        std::vector<char> _buffer; // 缓冲区
        uint32_t _readIndex;       // 读取Buffer缓冲数据起始处
        uint32_t _writeIndex;      // 写入Buffer缓冲数据起始处
    };
};

#endif //__MG_BUFFER_H__