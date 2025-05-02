#include "messageCache.h"
#include "../src/base/redis-connection-pool.h"

#include <sstream>

uint32_t MessageCache::getMessageId(uint32_t relationId)
{
    auto handle = mg::RedisPoolManager::get().getHandle("unRead");
    if (!handle)
        return 0;

    mg::RedisResult result;
    std::stringstream ss;
    ss << "message_id_" << relationId;
    if (!handle->INCR(ss.str(), result))
        return 0;

    return result;
}