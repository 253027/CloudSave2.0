#ifndef __CLIENT_CONNECTION_H__
#define __CLIENT_CONNECTION_H__

#include "../src/common/common.h"
#include "../src/base/tcp-connection.h"
#include "../src/protocol/imPduBase.h"

#include <string>
#include <list>
#include <tuple>

class ClientConnection : public ConnectionBase, public mg::TcpConnection
{
public:
    ClientConnection(mg::EventLoop *loop, const std::string &name, int sockfd,
                     const mg::InternetAddress &localAddress, const mg::InternetAddress &peerAddress);

    inline const std::string &getLoginName() const { return this->_loginName; }

    inline void setLoginName(const std::string &name) { this->_loginName = name; }

    inline uint32_t getUserId() { return this->_userId; }

    inline void setUserId(uint32_t id) { this->_userId = id; }

    inline void setValid() { this->_isValid = true; }

    inline bool isValid() const { return this->_isValid; }

    inline void setClientType(uint32_t type) { this->_clientType = type; }

    inline uint32_t getClientType() { return this->_clientType; }

    void send(const std::string &data);

    void addToSendList(uint32_t message_id, uint32_t other);

    void removeFromSendList(uint32_t message_id, uint32_t other);

    /**
     * @brief 更新用户状态至loginServer
     */
    void updateUserStatus(uint32_t status);

private:
    void connectionChangeCallback(const mg::TcpConnectionPointer &link) override;

    void writeCompleteCallback(const mg::TcpConnectionPointer &link) override;

    void messageCallback(const mg::TcpConnectionPointer &link, mg::Buffer *buf, mg::TimeStamp time) override;

private:
    void handleLoginRequest(const std::string &data);

    void handleGetLatestFriendList(const std::string &data);

    void handleSendMessage(const std::string &data);

    void handleTimeoutMessage();

    void handleSendMessageAck(const std::string &data);

private:
    std::string _loginName;                                         // 登录名
    uint32_t _userId;                                               // 登录ID
    uint32_t _clientType;                                           // 登录平台
    bool _isValid;                                                  // 连接是否经过验证
    uint16_t _sendPerSeconds;                                       // 每秒钟发送次数
    std::list<std::tuple<uint32_t, uint32_t, uint32_t>> _send_list; // 已发送但未收到确认的消息 message_id-other_id-timestamp
};

class ClientConnectionManger : public Singleton<ClientConnectionManger>
{
public:
    // ClientConnectionManger();

    void addConnection(const std::string &name, const mg::TcpConnectionPointer &connection); // FIXME: no thread safe

    void removeConnection(const std::string &name); // FIXME: no thread safe

    std::shared_ptr<ClientConnection> getConnctionByName(const std::string &name);

private:
    std::unordered_map<std::string, std::shared_ptr<mg::TcpConnection>> _memo;
};

#endif //__CLIENT_CONNECTION_H__