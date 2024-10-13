#ifndef __MG_CURRENT_THREAD_H__
#define __MG_CURRENT_THREAD_H__

#include <unistd.h>
#include <sys/syscall.h>

namespace mg
{
    namespace currentThread
    {
        extern __thread int _cachedTid; // 保存tid缓冲，避免多次系统调用

        void cacheTid();

        inline int tid()
        {
            if (__builtin_expect(_cachedTid == 0, 0))
                cacheTid();
            return _cachedTid;
        }
    };
};

#endif //__MG_CURRENT_THREAD_H__