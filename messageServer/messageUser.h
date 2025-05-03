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

    void removeUnvalidConnection(std::string &name);

    void addValidConnection(std::string name, std::weak_ptr<ClientConnection> connection);

    void removeValidConnection(std::string &name);

    void kickOutSameTypeUser(uint32_t type);

    std::weak_ptr<ClientConnection> getValidConnectionByName(const std::string &name);

    inline bool isValid() const { return this->_isValid; }

    inline void setValid() { this->_isValid = true; }

    inline uint16_t getValidConnectionCount() { return this->_connectionMemo.size(); }

    inline uint16_t getUnvalidConnectionCount() { return this->_unvalid.size(); }

    inline void setUserId(uint32_t uid) { this->_user_id = uid; }

    inline uint32_t getUserId() { return this->_user_id; }

    void boardcastData(const std::string &data, uint32_t message_id = 0, uint32_t other = 0);

private:
    bool _isValid;
    std::string _loginName;
    std::unordered_map<std::string, std::weak_ptr<ClientConnection>> _unvalid;
    std::unordered_map<std::string, std::weak_ptr<ClientConnection>> _connectionMemo;
    uint32_t _user_id;
};

class MessageUserManger : public Singleton<MessageUserManger>
{
public:
    bool addUserByUserName(std::string loginName, std::shared_ptr<MessageUser> user);

    void removeUserByUserName(std::string loginName);

    bool addUserByUserId(uint32_t uid, std::shared_ptr<MessageUser> user);

    void removeUserByUserId(uint32_t uid);

    void kickOutSameTypeUser(uint32_t uid, uint32_t type);

    std::shared_ptr<MessageUser> getUserByUserName(std::string username);

    std::shared_ptr<MessageUser> getUserByUserId(uint32_t uid);

    std::weak_ptr<ClientConnection> getConnectionByHandle(uint32_t uid, const std::string &name);

    uint16_t getUserConnectionCount();

private:
    std::unordered_map<uint32_t, std::shared_ptr<MessageUser>> _userMemoById;
    std::unordered_map<std::string, std::shared_ptr<MessageUser>> _userMemoByName;
};

#endif //__MESSAGE_USER_H__