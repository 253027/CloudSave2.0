#include "connector.h"
#include "event-loop.h"
#include "log.h"

const int mg::Connector::_maxRetryDelayMileSeconds = 30000;
const int mg::Connector::_initialRetryDelayMileSeconds = 500;

mg::Connector::Connector(int domain, int type, EventLoop *loop, const InternetAddress &address)
    : _loop(loop), _address(address), _state(DisConnected), _domain(domain), _type(type), _socket(0),
      _retryMileSeconds(_initialRetryDelayMileSeconds)
{
    ;
}

void mg::Connector::setNewConnectionCallback(const std::function<void(int sockfd)> &callback)
{
    this->_callback = callback;
}

void mg::Connector::start()
{
    this->_connect = true;
    this->_loop->run(std::bind(&Connector::startInLoop, this));
}

void mg::Connector::stop()
{
    this->_loop->run(std::bind(&Connector::stopInLoop, this));
}

void mg::Connector::restart()
{
    assert(this->_loop->isInOwnerThread());
    this->_connect = true;
    this->setState(DisConnected);
    this->startInLoop();
    this->_retryMileSeconds = _initialRetryDelayMileSeconds;
}

const mg::InternetAddress &mg::Connector::getAddress() const
{
    return _address;
}

void mg::Connector::startInLoop()
{
    assert(this->_loop->isInOwnerThread());
    assert(this->_state == DisConnected);
    if (this->_connect)
        connect();
    else
        LOG_DEBUG("EventLoop[{}] Connector do not connect", this->_loop->getLoopName());
}

void mg::Connector::stopInLoop()
{
    assert(this->_loop->isInOwnerThread());
    if (this->_state == Connecting)
    {
        this->setState(DisConnected);
        int sockfd = removeAndResetChannel();
    }
}

void mg::Connector::connect()
{
    int socket = createNonBlockScoket(this->_domain, this->_type);
    socklen_t len = 0;
    void *address = nullptr;
    if (_address.isIpv6())
    {
        len = sizeof(_address.getSockAddress_6());
        address = &_address.getSockAddress_6();
    }
    else
    {
        len = sizeof(_address.getSockAddress_4());
        address = &_address.getSockAddress_4();
    }
    int ret = ::connect(socket, (sockaddr *)address, len);
    int saveErrno = (ret == 0) ? 0 : errno;
    switch (saveErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting();
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry();
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        this->_socket.reset();
        break;
    }
}

void mg::Connector::connecting()
{
    this->setState(Connecting);
    assert(!this->_channel);
    this->_channel.reset(new Channel(_loop, _socket.fd()));
    this->_channel->setWriteCallback(std::bind(&Connector::handleWrite, this));
    this->_channel->setErrorCallback(std::bind(&Connector::handleError, this));
    this->_channel->enableWriting();
}

void mg::Connector::setState(State st)
{
    this->_state = st;
}

int mg::Connector::removeAndResetChannel()
{
    this->_channel->disableAllEvents();
    this->_channel->remove();
    int sockfd = this->_channel->fd();
    this->_loop->push(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void mg::Connector::resetChannel()
{
    if (this->_loop->hasChannel(this->_channel.get()))
        this->_channel->remove();
    this->_channel.reset();
}

void mg::Connector::handleWrite()
{
    int state = _state;
    LOG_TRACE("EventLoop[{}] handle write state: {}",
              this->_loop->getLoopName(), state);
    if (_state == Connecting)
    {
        // 这里这样做的目的是将连接成功的socket从当前channel中移除，
        // 加入TcpConnection中的channel进行维护
        int sockfd = removeAndResetChannel();
        int error = 0;
        socklen_t len = sizeof(error);
        ::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
        if (error)
            retry();
        else
        {
            this->setState(Connected);
            LOG_DEBUG("EventLoop[{}] connected to {}",
                      this->_loop->getLoopName(), _address.toIpPort());
            if (this->_connect && _callback)
                _callback(_socket.fd());
            else if (!this->_connect)
                _socket.reset();
        }
    }
    else
        assert(_state == DisConnected);
}

void mg::Connector::handleError()
{
    int state = _state;
    LOG_ERROR("EventLoop[{}] handel error state: {}",
              this->_loop->getLoopName(), state);
    if (_state == Connecting)
    {
        int error = 0;
        socklen_t len = sizeof(error);
        ::getsockopt(removeAndResetChannel(), SOL_SOCKET, SO_ERROR, &error, &len);
        LOG_TRACE("EventLoop[{}] SO_ERROR = {}",
                  this->_loop->getLoopName(), strerror(error));
        retry();
    }
}

void mg::Connector::retry()
{
    this->_socket.reset();
    this->setState(DisConnected);
    if (this->_connect)
    {
        LOG_INFO("EventLoop[{}] retry connnect to {}, next retrytime: {} seconds",
                 this->_loop->getLoopName(), _address.toIpPort(), _retryMileSeconds / 1000.0);
        _loop->runAfter(_retryMileSeconds / 1000.0, std::bind(&Connector::startInLoop, shared_from_this()));
        _retryMileSeconds = std::min(_retryMileSeconds * 2, Connector::_maxRetryDelayMileSeconds);
    }
    else
        LOG_DEBUG("EventLoop[{}] do not connect", this->_loop->getLoopName());
}

int mg::Connector::createNonBlockScoket(int domain, int type)
{
    this->_socket.setSocketType(domain, type);
    if (::fcntl(this->_socket.fd(), F_SETFL, ::fcntl(this->_socket.fd(), F_GETFL, 0) | O_NONBLOCK | O_CLOEXEC) < 0)
        LOG_ERROR("create nonblocksocket failed");
    return this->_socket.fd();
}
