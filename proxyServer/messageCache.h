#ifndef __MESSAGE_CACHE_H__
#define __MESSAGE_CACHE_H__

#include "../src/base/singleton.h"

#include <cstdint>

class MessageCache : public Singleton<MessageCache>
{
public:
    /**
     * @brief get unique message id
     * @return 0 if failed
     * @return > 0 if success
     */
    uint32_t getMessageId(uint32_t relationId);
};

#endif //  __MESSAGE_CACHE_H__