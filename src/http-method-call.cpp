#include "http-method-call.h"

// template <typename... Args>
// bool mg::HttpMethodCall::regist(const std::string &name, const std::string &path, BaseHandler<Args...> handler)
// {
//     if (!this->basic_type.count(name))
//         return false;
//     this->_functions[name][path] = std::move(handler);
//     return true;
// }

// template <typename... Args>
// bool mg::HttpMethodCall::exec(const std::string &name, const std::string &path, Args &&...args)
// {
//     auto it_name = this->_functions.find(name);
//     if (it_name == this->_functions.end())
//         return false;

//     auto it_path = it_name->second.find(path);
//     if (it_path == it_name->second.end())
//         return false;

//     it_path->second(std::forward<Args>(args)...);
//     return true;
// }
