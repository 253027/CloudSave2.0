#ifndef __MG_HTTP_PACKET_PARSER_H__
#define __MG_HTTP_PACKET_PARSER_H__

#include "singleton.h"
#include "function-callbacks.h"
#include "picohttpparser.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <map>

namespace mg
{
    using HttpHead = std::map<std::string, std::string>;
    using HttpBody = std::string;
    using HttpData = std::tuple<HttpHead, HttpBody>;

    class HttpPacketParser : public Singleton<HttpPacketParser>
    {
    public:
        HttpPacketParser();

        /**
         * @brief 按HTTP报文接受数据（前4字节为头部信息）
         * @param con TCP连接
         * @param data 待接收数据的存放容器
         */
        bool reveive(const mg::TcpConnectionPointer con, mg::HttpData &data);

    private:
    };

    std::string tolower(const std::string &str);
};

#endif //__MG_HTTP_PACKET_PARSER_H__