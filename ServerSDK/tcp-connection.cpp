#include "tcp-connection.h"
#include "event-loop.h"
#include "log.h"

mg::TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd,
                                 const InternetAddress &localAddress, const InternetAddress &peerAddress)
    : _loop(loop), _name(name), _socket(new Socket(sockfd)), _channel(new Channel(loop, sockfd)),
      _state(CONNECTING), _localAddress(localAddress), _peerAddress(peerAddress), _userStat(0)
{
    this->_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    this->_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    this->_channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    this->_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    this->_socket->setKeepLive(true);
}

mg::TcpConnection::~TcpConnection()
{
#ifdef _DEBUG
    LOG_TRACE("{} called ~TcpConnection()", this->_name);
#endif
    assert(_state == DISCONNECTED);
}

void mg::TcpConnection::setConnectionCallback(TcpConnectionCallback callback)
{
    this->_connectionCallback = std::move(callback);
}

void mg::TcpConnection::setMessageCallback(MessageDataCallback callback)
{
    this->_messageCallback = std::move(callback);
}

void mg::TcpConnection::setWriteCompleteCallback(WriteCompleteCallback callback)
{
    this->_writeCompleteCallback = std::move(callback);
}

void mg::TcpConnection::setCloseCallback(ConnectionClosedCallback callback)
{
    this->_closeCallback = std::move(callback);
}

void mg::TcpConnection::setHighWaterMarkCallback(HighWaterMarkCallback callback, int len)
{
    this->_highWaterCallback = std::move(callback);
    this->_highWaterMark = len;
}

bool mg::TcpConnection::connected()
{
    return this->_state == CONNECTED;
}

void mg::TcpConnection::shutdown()
{
    if (_state == CONNECTED)
    {
        this->setConnectionState(DISCONNECTED);
        _loop->run(std::bind(&TcpConnection::shutDownInOwnerLoop, this));
    }
}

void mg::TcpConnection::send(const std::string &data)
{
    if (_state != CONNECTED)
        return;
    if (_loop->isInOwnerThread())
        this->sendInOwnerLoop(data);
    else
        _loop->run(std::bind((void(TcpConnection::*)(const std::string &))(&TcpConnection::sendInOwnerLoop), this, data));
}

void mg::TcpConnection::send(Buffer &data)
{
    if (_state != CONNECTED)
        return;
    if (_loop->isInOwnerThread())
        this->sendInOwnerLoop(data.retrieveAllAsString());
    else
    {
        std::string buffer = data.retrieveAllAsString();
        _loop->run(std::bind((void(TcpConnection::*)(const std::string &))(&TcpConnection::sendInOwnerLoop), this, buffer));
    }
}

void mg::TcpConnection::connectionEstablished()
{
    // 这一部分是内置函数，需要将连接加入sub-eventloop中进行注册
    this->setConnectionState(CONNECTED);
    this->_channel->tie(shared_from_this());
    this->_channel->enableReading();

    // 执行用户自定义的连接建立时的回调
    if (this->_connectionCallback)
        this->_connectionCallback(shared_from_this());
}

void mg::TcpConnection::connectionDestoryed()
{
#ifdef _DEBUG
    LOG_TRACE("{} destroyed", this->_name);
#endif
    assert(_loop->isInOwnerThread());
    if (this->_state == CONNECTED)
    {
        this->setConnectionState(DISCONNECTED);
        this->_channel->disableAllEvents();
        if (this->_connectionCallback)
            this->_connectionCallback(shared_from_this());
    }
    this->_channel->remove();
}

void mg::TcpConnection::setUserConnectionState(int state)
{
    this->_userStat = state;
}

int mg::TcpConnection::getUserConnectionState()
{
    return this->_userStat;
}

void mg::TcpConnection::setConnectionState(State state)
{
    this->_state = state;
}

void mg::TcpConnection::handleRead(TimeStamp time)
{
    int saveErrno = 0;
    int len = this->_readBuffer.receive(this->_channel->fd(), saveErrno);
    if (len > 0)
    {
        if (this->_messageCallback)
            this->_messageCallback(shared_from_this(), &this->_readBuffer, time);
        else
            LOG_ERROR("{} _messageCallback not initialized", this->_name);
    }
    else if (len == 0)
        this->handleClose();
    else
    {
        errno = saveErrno;
        LOG_ERROR("{} {}", this->_name, ::strerror(errno));
        this->handleError();
    }
}

void mg::TcpConnection::handleClose()
{
    this->setConnectionState(DISCONNECTED);
    this->_channel->disableAllEvents();
    TcpConnectionPointer temp(shared_from_this());
    if (this->_connectionCallback)
        this->_connectionCallback(temp);
    else
        LOG_WARN("{} _connectionCallback not initial", this->_name);
    if (this->_closeCallback)
        this->_closeCallback(temp);
    else
        LOG_WARN("{} _closeCallback not initial", this->_name);
}

void mg::TcpConnection::handleError()
{
    int option, saveError = 0;
    socklen_t len = sizeof(option);
    if (::getsockopt(_channel->fd(), SOL_SOCKET, SO_ERROR, &option, &len) < 0)
        saveError = errno;
    else
        saveError = option;
    LOG_ERROR("{} {}", this->_name, ::strerror(saveError));
}

void mg::TcpConnection::handleWrite()
{
    if (this->_channel->isWriting())
    {
        int saveError = 0;
        int len = _sendBuffer.send(_channel->fd(), saveError);
        if (len > 0)
        {
            _sendBuffer.retrieve(len);
            if (_sendBuffer.readableBytes() == 0)
            {
                _channel->disableWriting(); // 取消写事件
                if (_writeCompleteCallback)
                    _loop->push(std::bind(_writeCompleteCallback, shared_from_this()));
                if (_state == DISCONNECTING)
                    this->shutDownInOwnerLoop();
            }
        }
        else
            LOG_ERROR("{} {}", this->_name, ::strerror(saveError));
    }
    else
        LOG_ERROR("{} don't need to send", this->_name);
}

void mg::TcpConnection::shutDownInOwnerLoop()
{
    if (!_channel->isWriting())
        _socket->shutDownWrite();
}

void mg::TcpConnection::sendInOwnerLoop(const void *data, int len)
{
    bool hasError = false;
    int remain = len, hasWrite = 0;
    if (_state == DISCONNECTED)
    {
        LOG_ERROR("[{}] disconnected");
        return;
    }

    // 没有注册可写事件并且发送缓冲区为空
    if (!_channel->isWriting() && _sendBuffer.readableBytes() == 0)
    {
        hasWrite = ::write(_channel->fd(), data, len);
        if (hasWrite >= 0)
        {
            remain = len - hasWrite;
            if (!remain && _writeCompleteCallback)
                _loop->push(std::bind(_writeCompleteCallback, shared_from_this()));
        }
        else
        {
            hasWrite = 0;
            if ((errno != EWOULDBLOCK) && (errno == EPIPE || errno == ECONNRESET))
            {
                hasError = true;
                LOG_ERROR("{} Error: {}", this->_name, ::strerror(errno));
            }
        }
    }

    /**
     * 说明当前这一次write并没有把数据全部发送出去 剩余的数据需要保存到缓冲区当中
     * 然后给channel注册EPOLLOUT事件，Poller发现tcp的发送缓冲区有空间后会通知
     * 相应的sock->channel，调用channel对应注册的writeCallback_回调方法，
     * channel的writeCallback_实际上就是TcpConnection设置的handleWrite回调，
     * 把发送缓冲区outputBuffer_的内容全部发送完成
     **/
    if (!hasError && remain)
    {
        int last = _sendBuffer.readableBytes();

        if (last + remain >= _highWaterMark && last < _highWaterMark && _highWaterCallback)
            _loop->push(std::bind(_highWaterCallback, shared_from_this(), last + remain));

        _sendBuffer.append((char *)data + hasWrite, remain);
        if (!_channel->isWriting())
            _channel->enableWriting();
    }
}

void mg::TcpConnection::sendInOwnerLoop(const std::string &data)
{
    this->sendInOwnerLoop(data.data(), data.size());
}
