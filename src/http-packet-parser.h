#ifndef __MG_HTTP_PACKET_PARSER_H__
#define __MG_HTTP_PACKET_PARSER_H__

#include "singleton.h"
#include "function-callbacks.h"
#include "picohttpparser.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <unordered_map>

namespace mg
{
    using HttpHead = std::unordered_map<std::string, std::string>;
    using HttpBody = std::string;
    using HttpData = std::tuple<HttpHead, HttpBody>;

    class HttpRequest
    {
    public:
        const std::string &method() const;

        const std::string &path() const;

        const std::string &getHeader(const std::string &key) const;

        const std::string body() const;

    private:
        friend class HttpPacketParser;
        std::string _method;
        std::string _path;
        std::unordered_map<std::string, std::string> _headers;
        std::string _body;
    };

    class HttpResponse
    {
    public:
        void setStatus(int status);

        void setHeader(const std::string &key, const std::string &value);

        void setBody(const std::string &body);

        std::string dump() const;

    private:
        int _status;
        std::unordered_map<std::string, std::string> _headers;
        std::string _body;
    };

    class HttpPacketParser : public Singleton<HttpPacketParser>
    {
    public:
        HttpPacketParser();

        /**
         * @brief 按HTTP报文接受数据
         * @param con TCP连接
         * @param data 待接收数据的存放容器
         */
        bool reveive(const mg::TcpConnectionPointer con, mg::HttpRequest &data);

        bool send(const mg::TcpConnectionPointer con, mg::HttpResponse &data);

        int parseType(const std::string &data);

    private:
    public:
        std::unordered_map<std::string, std::unordered_map<std::string, int>> HttpContentType =
            {
                {
                    "text",
                    {
                        {"plain", 1},      // 纯文本
                        {"html", 2},       // HTML文档
                        {"css", 3},        // CSS 样式表
                        {"javascript", 4}, // JavaScript 脚本
                        {"csv", 5},        // 逗号分隔值文件（CSV）
                        {"xml", 6},        // XML文档
                    },
                },
                {
                    "application",
                    {
                        {"json", 7},                  // JSON数据
                        {"xml", 8},                   // xml数据
                        {"x-www-form-urlencoded", 9}, // URL 编码的表单数据
                        {"octet-stream", 10},         // 未知的二进制数据，常用于文件下载
                    },
                },
                // ToDo:
            };
    };

    std::string tolower(const std::string &str);
};

#endif //__MG_HTTP_PACKET_PARSER_H__