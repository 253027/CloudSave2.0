#include "messageUser.h"
#include "clientConnection.h"

bool MessageUserManger::addUserByUserName(std::string loginName, std::shared_ptr<MessageUser> user)
{
    if (this->_userMemoByName.count(loginName))
        return false;
    this->_userMemoByName[loginName] = std::move(user);
    return true;
}

void MessageUserManger::removeUserByUserName(std::string loginName)
{
    this->_userMemoByName.erase(loginName);
}

bool MessageUserManger::addUserByUserId(uint32_t uid, std::shared_ptr<MessageUser> user)
{
    if (this->_userMemoById.count(uid))
        return false;
    this->_userMemoById[uid] = std::move(user);
    return true;
}

void MessageUserManger::removeUserByUserId(uint32_t uid)
{
    this->_userMemoById.erase(uid);
}

void MessageUserManger::kickOutSameTypeUser(uint32_t uid, uint32_t type)
{
    auto it = this->_userMemoById.find(uid);
    if (it == this->_userMemoById.end())
        return;
    it->second->kickOutSameTypeUser(type);
}

std::shared_ptr<MessageUser> MessageUserManger::getUserByUserName(std::string username)
{
    auto it = this->_userMemoByName.find(username);
    if (it == this->_userMemoByName.end())
        return std::shared_ptr<MessageUser>();
    return it->second;
}

std::shared_ptr<MessageUser> MessageUserManger::getUserByUserId(uint32_t uid)
{
    auto it = this->_userMemoById.find(uid);
    if (it == this->_userMemoById.end())
        return std::shared_ptr<MessageUser>();
    return it->second;
}

std::weak_ptr<ClientConnection> MessageUserManger::getConnectionByHandle(uint32_t uid, const std::string &name)
{
    auto it = this->_userMemoById.find(uid);
    if (it == this->_userMemoById.end())
        return std::weak_ptr<ClientConnection>();
    return it->second->getValidConnectionByName(name);
}

uint16_t MessageUserManger::getUserConnectionCount()
{
    uint16_t ret = 0;
    for (auto &x : this->_userMemoById)
        ret += x.second->getValidConnectionCount();
    return ret;
}

MessageUser::MessageUser(const std::string &loginName)
    : _isValid(false), _loginName(loginName)
{
    ;
}

void MessageUser::addUnValidConnection(std::string name, std::weak_ptr<ClientConnection> connection)
{
    this->_unvalid[std::move(name)] = std::move(connection);
}

void MessageUser::removeUnvalidConnection(std::string &name)
{
    this->_unvalid.erase(name);
}

void MessageUser::addValidConnection(std::string name, std::weak_ptr<ClientConnection> connection)
{
    this->_connectionMemo[std::move(name)] = connection;
}

void MessageUser::removeValidConnection(std::string &name)
{
    this->_connectionMemo.erase(name);
}

void MessageUser::kickOutSameTypeUser(uint32_t type)
{
    PduMessage pdu;
    pdu.setCommandId(IM::BaseDefine::COMMAND_ID_OTHER_SERVER_KICK_USER);
    pdu.setServiceId(IM::BaseDefine::SERVER_ID_OTHER);

    for (auto it = this->_connectionMemo.begin(); it != this->_connectionMemo.end();)
    {
        auto connection = it->second.lock();
        if (connection->getClientType() == type)
        {
            connection->setUserConnectionState(1);
            connection->send(pdu.dump());
            it = this->_connectionMemo.erase(it);
        }
        else
            it++;
    }
}

std::weak_ptr<ClientConnection> MessageUser::getValidConnectionByName(const std::string &name)
{
    auto it = this->_connectionMemo.find(name);
    if (it == this->_connectionMemo.end())
        return std::weak_ptr<ClientConnection>();
    return it->second;
}

void MessageUser::boardcastData(const std::string &data, uint32_t message_id, uint32_t other)
{
    for (auto &connection : this->_connectionMemo)
    {
        auto link = connection.second.lock();
        if (!link)
            continue;
        if (message_id && other)
            link->addToSendList(message_id, other);
        link->send(data);
    }
}