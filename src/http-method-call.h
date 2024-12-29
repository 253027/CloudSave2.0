#ifndef __MG_HTTP_METHOD_CALL_H__
#define __MG_HTTP_METHOD_CALL_H__

#include "singleton.h"
#include "function-callbacks.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <tuple>

namespace mg
{
    // class HttpMethodCall : public Singleton<HttpMethodCall>
    // {
    // public:
    //     template <typename... Args>
    //     bool regist(const std::string &name, const std::string &path, BaseHandler<Args...> handler);

    //     template <typename... Args>
    //     bool exec(const std::string &name, const std::string &path, Args &&...args);

    // private:
    //     template <typename F, typename Tuple, std::size_t... I>
    //     void call_handler(F &&f, Tuple &&t, std::index_sequence<I...>)
    //     {
    //         f(std::get<I>(std::forward<Tuple>(t))...); // 解包并调用 handler
    //     }

    //     template <typename F, typename Tuple>
    //     void call_helper(F &&f, Tuple &&t)
    //     {
    //         constexpr std::size_t size = std::tuple_size<typename std::decay<Tuple>::type>::value;
    //         call_handler(std::forward<F>(f), std::forward<Tuple>(t), std::make_index_sequence<size>{});
    //     }

    //     using common = std::function<void(std::tuple<void>)>;
    //     std::unordered_map<std::string, std::unordered_map<std::string, common>> _functions;

    //     const std::unordered_set<std::string> basic_type =
    //         {
    //             "GET",
    //             "POST", //
    //         };
    // };
};

#endif //__MG_HTTP_METHOD_CALL_H__