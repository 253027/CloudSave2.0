#ifndef __MG_FUNCTION_CALLBACK_H__
#define __MG_FUNCTION_CALLBACK_H__

#include "time-stamp.h"

#include <functional>
#include <memory>

namespace mg
{
    class Buffer;
    class TcpConnection;
    using TcpConnectionPointer = std::shared_ptr<TcpConnection>;
    using TcpConnectionCallback = std::function<void(const TcpConnectionPointer &)>;
    using MessageDataCallback = std::function<void(const TcpConnectionPointer &, Buffer *, TimeStamp)>;
    using WriteCompleteCallback = std::function<void(const TcpConnectionPointer &)>;
    using ConnectionClosedCallback = std::function<void(const TcpConnectionPointer &)>;
    using HighWaterMarkCallback = std::function<void(const TcpConnectionPointer &, int)>;
}

#endif //__MG_FUNCTION_CALLBACK_H__