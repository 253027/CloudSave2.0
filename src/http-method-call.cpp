#include "http-method-call.h"

bool mg::HttpMethodCall::regist(const std::string &name, const std::string &path, HttpHandler handler)
{
    if (!this->basic_type.count(name))
        return false;
    this->_functions[name][path] = std::move(handler);
    return true;
}

bool mg::HttpMethodCall::regist2(const std::string &name, const std::string &path, HttpHandler handler)
{
    if (!this->basic_type.count(name))
        return false;
    this->_regexFunction[name].emplace_back(std::regex(path), std::move(handler));
    return true;
}

bool mg::HttpMethodCall::exec(const HttpRequest &request)
{
    return this->exec(request.method(), request.path(), request);
}

bool mg::HttpMethodCall::exec(const std::string &name, const std::string &path, const HttpRequest &request)
{
    auto it_name = this->_functions.find(name);
    if (it_name != this->_functions.end())
    {
        auto it_path = it_name->second.find(path);
        if (it_path != it_name->second.end())
            return it_path->second(request);
    }

    auto it_regex = this->_regexFunction.find(name);
    if (it_regex != this->_regexFunction.end())
    {
        auto &functions = it_regex->second;
        for (auto &x : functions)
        {
            if (std::regex_match(path, x.first))
                return x.second(request);
        }
    }

    return false;
}
