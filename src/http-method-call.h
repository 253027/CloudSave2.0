#ifndef __MG_HTTP_METHOD_CALL_H__
#define __MG_HTTP_METHOD_CALL_H__

#include "singleton.h"
#include "function-callbacks.h"
#include "http-packet-parser.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <tuple>
#include <regex>

namespace mg
{
    class HttpMethodCall : public Singleton<HttpMethodCall>
    {
        using HttpHandler = std::function<bool(const HttpRequest &)>;

    public:
        bool regist(const std::string &name, const std::string &path, HttpHandler handler);

        bool regist2(const std::string &name, const std::string &path, HttpHandler handler);

        bool exec(const HttpRequest &request);

        bool exec(const std::string &name, const std::string &path, const HttpRequest &request);

    private:
        std::unordered_map<std::string, std::unordered_map<std::string, HttpHandler>> _functions;
        std::unordered_map<std::string, std::vector<std::pair<std::regex, HttpHandler>>> _regexFunction;

        const std::unordered_set<std::string> basic_type =
            {
                "GET",
                "POST", //
            };
    };
};

#endif //__MG_HTTP_METHOD_CALL_H__