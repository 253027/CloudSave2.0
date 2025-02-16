#ifndef __MESSAGE_USER_H__
#define __MESSAGE_USER_H__

#include "../src/base/singleton.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>

class ClientConnection;

class MessageUser
{
public:
    MessageUser(const std::string &loginName);

    void addUnValidConnection(std::string name, std::weak_ptr<ClientConnection> connection);

private:
    std::string _loginName;
    std::unordered_map<std::string, std::weak_ptr<ClientConnection>> _unvalid;
};

class MessageUserManger : public Singleton<MessageUserManger>
{
public:
    bool addUserByUserName(std::string loginName, std::shared_ptr<MessageUser> user);

    void removeUserByUserName(std::string loginName);

    bool addUserByUserId(uint32_t uid, std::shared_ptr<MessageUser> user);

    void removeUserByUserId(uint32_t uid);

    std::shared_ptr<MessageUser> getUserByUserName(std::string username);

    std::shared_ptr<MessageUser> getUserByUserId(uint32_t uid);

private:
    std::unordered_map<uint32_t, std::shared_ptr<MessageUser>> _userMemoById;
    std::unordered_map<std::string, std::shared_ptr<MessageUser>> _userMemoByName;
};

#endif //__MESSAGE_USER_H__