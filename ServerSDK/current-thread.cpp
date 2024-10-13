#include "current-thread.h"

namespace mg
{
    namespace currentThread
    {
        __thread int _cachedTid = 0;

        void cacheTid()
        {
            if (_cachedTid == 0)
                _cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    };
};
