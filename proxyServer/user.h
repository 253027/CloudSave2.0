#ifndef __USER_H__
#define __USER_H__

#include "../src/base/singleton.h"
#include "../src/common/common.h"

#include <cstdint>
#include <vector>

class User : public Singleton<User>
{
public:
    bool getFriendsList(uint32_t userId, std::vector<uint32_t> &list, uint32_t lastUpdateTime = 0);

    bool getFriendsInfo(uint32_t userId, IM::DataStruct::UserInformation &info);
};

#endif //__USER_H__