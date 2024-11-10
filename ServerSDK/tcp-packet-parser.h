#ifndef __MG_TCP_PACKET_PARSER__
#define __MG_TCP_PACKET_PARSER__

#include "singleton.h"
#include "function-callbacks.h"

namespace mg
{
    class TcpPacketParser : public Singleton<mg::TcpPacketParser>
    {
    public:
        /**
         * @brief 按边界发送数据（加上4字节指定data数据长度）
         * @param con 待发送链接
         * @param data 待发送数据
         */
        bool send(const mg::TcpConnectionPointer con, const std::string &data);

        /**
         * @brief 按边界接受数据（前4字节为头部信息）
         * @param con 待发送链接
         * @param data 待接收数据的存放容器
         */
        bool reveive(const mg::TcpConnectionPointer con, std::string &data);

    private:
        int headSize = 4;
    };
};

#endif //__MG_TCP_PACKET_PARSER__