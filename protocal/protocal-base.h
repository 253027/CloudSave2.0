#ifndef __PROTOCAL_BASE_H__
#define __PROTOCAL_BASE_H__

#include "../src/macros.h"

#include <cstdint>

namespace Protocal
{
    using TLV = std::tuple<int, std::string>;

    enum class BaseCommandType : uint16_t
    {
        SESSION_SERVER_BASE = 1,
    };

    struct Command
    {
        Command() : base_type(0)
        {
            ;
        };

        Command(int type) : base_type(type)
        {
            ;
        }

        int16_t base_type;
    };
};

#endif //__PROTOCAL_H__