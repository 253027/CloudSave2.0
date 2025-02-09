#include "http-packet-parser.h"
#include "tcp-connection.h"
#include <sstream>
#include <iomanip>

mg::HttpPacketParser::HttpPacketParser()
{
    ;
}

bool mg::HttpPacketParser::reveive(const mg::TcpConnectionPointer con, mg::HttpRequest &data)
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

    data._method = std::string(method, method_len);
    data._path = urlDecode(std::string(path, path_len));
    for (int i = 0; i < num_headers; i++)
        data._headers[mg::tolower(std::string(headers[i].name, headers[i].name_len))] = urlDecode(std::string(headers[i].value, headers[i].value_len));

    int body_size = 0;
    auto it = data._headers.find("content-length");
    if (it != data._headers.end())
        body_size = std::stoi(it->second);
    if (con->_readBuffer.readableBytes() < ret + body_size)
        return false;

    data._body = con->_readBuffer.retrieveAsString(ret + body_size).substr(ret);
    return true;
}

bool mg::HttpPacketParser::send(const mg::TcpConnectionPointer con, mg::HttpResponse &data)
{
    con->send(data.dump());
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

std::vector<std::string> mg::spilt(const std::string &str, const std::string &delimiter)
{
    if (delimiter.empty())
        return {str};
    std::vector<std::string> ret;

    size_t start = 0, end;
    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        std::string temp = str.substr(start, end - start);
        if (!temp.empty())
            ret.emplace_back(temp);
        start = end + delimiter.size();
    }
    std::string temp = str.substr(start);
    if (!temp.empty())
        ret.push_back(temp);
    return ret;
}

std::string mg::urlDecode(const std::string &str)
{
    std::string result;
    for (int i = 0; i < str.size();)
    {
        if (str[i] == '%')
        {
            int value = 0;
            std::istringstream(str.substr(i + 1, 2)) >> std::hex >> value;
            result += static_cast<char>(value);
            i += 3;
        }
        else if (str[i] == '+')
        {
            result += ' ';
            i++;
        }
        else
        {
            result += str[i];
            i++;
        }
    }
    return result;
}

std::string mg::urlEncode(const std::string &str)
{
    std::ostringstream os;
    for (auto &c : str)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            os << c;
        else
            os << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return os.str();
}

void mg::HttpResponse::setStatus(int status)
{
    this->_status = status;
}

void mg::HttpResponse::setHeader(const std::string &key, const std::string &value)
{
    this->_headers[key] = value;
}

void mg::HttpResponse::setBody(const std::string &body)
{
    this->_body = body;
}

std::string mg::HttpResponse::dumpHead() const
{
    std::stringstream response;
    response << "HTTP/1.1 " << _status << "\r\n";
    for (auto &val : _headers)
        response << val.first << ": " << val.second << "\r\n";
    return response.str();
}

std::string mg::HttpResponse::dump() const
{
    std::stringstream response;
    response << this->dumpHead() << "Content-Length: "
             << std::to_string(_body.size()) << "\r\n\r\n"
             << _body;
    return response.str();
}

mg::HttpRequest::HttpRequest(const mg::TcpConnectionPointer &con)
{
    this->addParam(con);
}

const std::string &mg::HttpRequest::method() const
{
    return this->_method;
}

const std::string &mg::HttpRequest::path() const
{
    return this->_path;
}

bool mg::HttpRequest::hasHeader(const std::string &key) const
{
    return this->_headers.count(key);
}

const std::string &mg::HttpRequest::getHeader(const std::string &key) const
{
    static std::string memo;
    auto it = this->_headers.find(key);
    if (it == this->_headers.end())
        return memo;
    return it->second;
}

const std::string &mg::HttpRequest::body() const
{
    return this->_body;
}

const mg::TcpConnectionPointer mg::HttpRequest::getConnection() const
{
    return getParam<mg::TcpConnectionPointer>(0);
}
