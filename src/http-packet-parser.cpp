#include "http-packet-parser.h"
#include "tcp-connection.h"
#include <sstream>

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
    head["method_path"] = std::string(path, path_len);
    for (int i = 0; i < num_headers; i++)
        head[mg::tolower(std::string(headers[i].name, headers[i].name_len))] = mg::tolower(std::string(headers[i].value, headers[i].value_len));

    int body_size = 0;
    auto it = head.find("content-length");
    if (it != head.end())
        body_size = std::stoi(it->second);
    body = con->_readBuffer.retrieveAsString(ret + body_size).substr(ret);
    return true;
}

bool mg::HttpPacketParser::send(const mg::TcpConnectionPointer con, mg::HttpData &data)
{
    std::stringstream response;

    HttpHead head;
    HttpBody body;
    std::tie(head, body) = std::move(data);

    if (!head.count("HTTP/1.1"))
        return false;

    response << "HTTP/1.1 " << head["HTTP/1.1"] << "\r\n";
    for (auto &val : head)
    {
        if (val.first == "HTTP/1.1")
            continue;
        response << val.first << ": " << val.second << "\r\n";
    }
    response << "Content-Length: " << std::to_string(body.size()) << "\r\n";
    response << "\r\n"
             << body;

    con->send(response.str());
    return true;
}

int mg::HttpPacketParser::parseType(const std::string &data)
{
    auto it = data.find("/");
    if (it == std::string::npos)
        return 0;
    std::string type_1 = data.substr(0, it);
    std::string type_2 = data.substr(it + 1);
    if (!HttpContentType.count(type_1) || !HttpContentType[type_1].count(type_2))
        return 0;
    return HttpContentType[type_1][type_2];
}

std::string mg::tolower(const std::string &str)
{
    std::string res = str;
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c)
                   { return std::tolower(c); });
    return std::move(res);
}
