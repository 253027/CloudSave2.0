#ifndef __PROTOCAL_TYPE_H__
#define __PROTOCAL_TYPE_H__

#include <tuple>
#include <string>

namespace Protocal
{
    using TLV = std::tuple<int, std::string>;

    enum PackageType
    {
        BASIC_BYTES = 1,  // 基本二进制数据
        BASIC_STRING = 2, // 基本字符串数据
        JSON_STRING = 3,  // JSON文本数据
    };

    inline TLV parse(std::string &data)
    {
        if (data.size() < 4)
            return {};
        int type = *((int *)data.data());
        std::string ret_data(data.begin() + 4, data.end());
        return std::make_tuple(type, ret_data);
    }
};

#endif //__PROTOCAL_TYPE_H__