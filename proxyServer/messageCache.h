#ifndef __MESSAGE_CACHE_H__
#define __MESSAGE_CACHE_H__

#include "../src/base/singleton.h"
#include "../src/common/common.h"

#include <cstdint>

class MessageCache : public Singleton<MessageCache>
{
    using UnReadMessage = IM::DataStruct::UnReadMessage;

public:
    /**
     * @brief get unique message id
     * @return 0 if failed
     * @return > 0 if success
     */
    uint32_t getMessageId(uint32_t relationId);

    void setUnReadMessage(uint32_t userId, uint32_t relationId, uint32_t messageId);
    void getUnReadMessage(uint32_t userId, std::vector<UnReadMessage> &message);

    void setReadMessage(uint32_t relationId, uint32_t messageId);
};

#endif //  __MESSAGE_CACHE_H__