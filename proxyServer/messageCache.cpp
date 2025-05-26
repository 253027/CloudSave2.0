#include "messageCache.h"
#include "../src/base/log.h"
#include "../src/base/redis-connection-pool.h"

#include <sstream>

uint32_t MessageCache::getMessageId(uint32_t relationId)
{
    auto handle = mg::RedisPoolManager::get().getHandle("unRead");
    if (!handle)
    {
        LOG_ERROR("get redis handle failed");
        return 0;
    }

    mg::RedisResult result;
    std::stringstream ss;
    ss << "message_id_" << relationId;
    if (!handle->INCR(ss.str(), result))
    {
        LOG_ERROR("realtion: {}, redis INCR failed: {}", relationId, result.str);
        return 0;
    }

    return result;
}

void MessageCache::setUnReadMessage(uint32_t userId, uint32_t relationId, uint32_t messageId)
{
    auto handle = mg::RedisPoolManager::get().getHandle("unRead");
    if (!handle)
    {
        LOG_ERROR("get redis handle failed");
        return;
    }
    std::string key = "un_read_user_" + std::to_string(userId);
    if (!handle->HSET(key, std::to_string(relationId), std::to_string(messageId)))
        LOG_ERROR("redis HSET failed {}, {}, {}", key, relationId, messageId);
}

void MessageCache::getUnReadMessage(uint32_t userId, std::vector<UnReadMessage> &message)
{
    auto handle = mg::RedisPoolManager::get().getHandle("unRead");
    if (!handle)
    {
        LOG_ERROR("get redis handle failed");
        return;
    }

    mg::RedisResult result;

    std::string key = "un_read_user_" + std::to_string(userId);
    if (!handle->HGETALL(key, result))
        return;
    handle.reset();

    std::vector<std::string> values = result;
    for (int i = 0, len = values.size(); i < len; i += 2)
    {
        UnReadMessage temp;
        temp.set_session_id(std::stoi(values[i]));
        temp.set_latest_sequence(std::stoi(values[i + 1]));
    }
}