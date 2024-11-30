#ifndef __MG_MACROS_H__
#define __MG_MACROS_H__

#define SAFE_DELETE(x)      \
    do                      \
    {                       \
        if ((x) != nullptr) \
        {                   \
            delete (x);     \
            (x) = nullptr;  \
        }                   \
    } while (0)

#define TO_UNDERLYING(x) static_cast<std::underlying_type_t<decltype(x)>>(x)

#define TO_ENUM(enumType, x) static_cast<enumType>(x)

#endif //__MG_MACROS_H__