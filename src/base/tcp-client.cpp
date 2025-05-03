#include "tcp-client.h"
#include "tcp-connection.h"
#include "event-loop.h"
#include "connector.h"

#include "log.h"

namespace mg
{
    void removeConnection(EventLoop *loop, const TcpConnectionPointer &connection)
    {
        loop->push(std::bind(&TcpConnection::connectionDestoryed, connection));
    }
};

mg::TcpClient::TcpClient(int domain, int type, EventLoop *loop, const InternetAddress &address, const std::string &name)
    : _loop(loop), _name(name), _connected(false), _connector(new Connector(domain, type, _loop, address)),
      _isIpv6(address.isIpv6()), _connectionID(0), _retry(false)
{
    _connector->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

mg::TcpClient::~TcpClient()
{
    if (!_connected)
        return;

    LOG_DEBUG("{} called ~TcpClient()", this->_connector->getAddress().toIpPort());
    TcpConnectionPointer connection;
    bool unique = false;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        unique = _connection.unique();
        connection = _connection;
    }
    if (connection)
    {
        assert(_loop == connection->getLoop());
        ConnectionClosedCallback callback = std::bind(&mg::removeConnection, _loop, std::placeholders::_1);
        _loop->run(std::bind(&TcpConnection::setCloseCallback, connection, callback));
        if (unique)
            connection->forceClose();
    }
    else
    {
        _connector->stop();
    }
}

void mg::TcpClient::connect()
{
    LOG_DEBUG("connecting to {}", _connector->getAddress().toIpPort());
    _connected = true;
    _connector->start();
}

void mg::TcpClient::stop()
{
    _connected = false;
    if (_connection)
        _connection->forceClose();
    _connector->stop();
}

void mg::TcpClient::disconnect()
{
    _connected = false;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        if (_connection)
            _connection->shutdown();
    }
}

void mg::TcpClient::enableRetry()
{
    this->_retry = true;
}

void mg::TcpClient::disableRetry()
{
    this->_retry = false;
}

void mg::TcpClient::setConnectionCallback(TcpConnectionCallback callback)
{
    _connectionCallback = std::move(callback);
}

void mg::TcpClient::setMessageCallback(MessageDataCallback callback)
{
    _messgageDataCallback = std::move(callback);
}

void mg::TcpClient::setWriteCompleteCallback(WriteCompleteCallback callback)
{
    _writeCompleteCallback = std::move(callback);
}

bool mg::TcpClient::connected()
{
    return _connection && _connection->connected();
}

mg::TcpConnectionPointer mg::TcpClient::connection()
{
    std::lock_guard<std::mutex> guard(_mutex);
    return this->_connection;
}

void mg::TcpClient::newConnection(int sockfd)
{
    assert(_loop->isInOwnerThread());
    InternetAddress localAddress(mg::Socket::getLocalAddress(sockfd));
    InternetAddress peerAddress(mg::Socket::getPeerAddress(sockfd));
    char buf[1024] = {0};
    snprintf(buf, sizeof(buf), "-%s#%d", localAddress.toIpPort().c_str(), ++_connectionID);
    std::string connectionName = _name + buf;

    TcpConnectionPointer connection(new TcpConnection(_loop, connectionName, sockfd, localAddress, peerAddress));
    connection->setWriteCompleteCallback(_writeCompleteCallback);
    connection->setMessageCallback(_messgageDataCallback);
    connection->setConnectionCallback(_connectionCallback);
    connection->setCloseCallback(std::bind(&TcpClient::removeConntction, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> guard(_mutex);
        this->_connection = connection;
    }
    connection->connectionEstablished();
}

void mg::TcpClient::removeConntction(const TcpConnectionPointer &connection)
{
    assert(_loop->isInOwnerThread());
    assert(_loop == connection->getLoop());
    {
        std::lock_guard<std::mutex> guard(_mutex);
        assert(connection == _connection);
        _connection.reset();
    }
    _loop->push(std::bind(&TcpConnection::connectionDestoryed, connection));
    if (_retry && _connected)
    {
        LOG_INFO("client reconnect to {}", _connector->getAddress().toIpPort());
        _connector->restart();
    }
}
