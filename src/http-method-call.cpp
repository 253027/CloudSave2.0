#include "http-method-call.h"

bool mg::HttpMethodCall::regist(const std::string &name, const std::string &path, HttpHandler handler)
{
    if (!this->basic_type.count(name))
        return false;
    this->_functions[name][path] = std::move(handler);
    return true;
}

bool mg::HttpMethodCall::exec(const HttpRequest &request)
{
    return this->exec(request.method(), request.path(), request);
}

bool mg::HttpMethodCall::exec(const std::string &name, const std::string &path, HttpRequest request)
{
    auto it_name = this->_functions.find(name);
    if (it_name == this->_functions.end())
        return false;

    auto it_path = it_name->second.find(path);
    if (it_path == it_name->second.end())
        return false;

    it_path->second(request);
    return true;
}
