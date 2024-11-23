#include "http-packet-parser.h"
#include "tcp-connection.h"

mg::HttpPacketParser::HttpPacketParser()
{
    ;
}

bool mg::HttpPacketParser::reveive(const mg::TcpConnectionPointer con, mg::HttpData &data)
{
    const char *method;
    const char *path;
    int minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    num_headers = sizeof(headers) / sizeof(headers[0]);
    int ret = phr_parse_request(con->_readBuffer.readPeek(), con->_readBuffer.readableBytes(),
                                &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, 1);
    if (ret < 0)
        return false;

    HttpHead &head = std::get<0>(data);
    HttpBody &body = std::get<1>(data);
    head["method"] = std::string(method, method_len);
    for (int i = 0; i < num_headers; i++)
        head[mg::tolower(std::string(headers[i].name, headers[i].name_len))] = std::string(headers[i].value, headers[i].value_len);

    int body_size = 0;
    auto it = head.find("content-length");
    if (it != head.end())
        body_size = std::stoi(it->second);
    body = con->_readBuffer.retrieveAsString(ret + body_size).substr(ret);
    return true;
}

std::string mg::tolower(const std::string &str)
{
    std::string res = str;
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c)
                   { return std::tolower(c); });
    return std::move(res);
}
