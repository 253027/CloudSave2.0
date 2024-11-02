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

#endif //__MG_MACROS_H__