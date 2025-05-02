#include "acceptor.h"
#include "socket.h"
#include "log.h"
#include "event-loop.h"

#include <fcntl.h>

mg::Acceptor::Acceptor(int domain, int type, EventLoop *loop, const InternetAddress &listenAddress, bool reusePort)
    : _loop(loop), _socket(0), _channel(loop, createNonBlockScoket(domain, type)),
      _vacantFd(::open("/dev/null", O_RDWR | O_CLOEXEC))
{
    _socket.setReuseAddress(true);
    _socket.bind(listenAddress);
    _channel.setReadCallback(std::bind(&Acceptor::handleReadEvent, this));
    LOG_DEBUG("EventLoop[{}] Acceptor Socket fd is {}", this->_loop->getLoopName(), this->_socket.fd());
}

mg::Acceptor::~Acceptor()
{
    this->_channel.disableAllEvents();
    this->_channel.remove();
    TEMP_FAILURE_RETRY(::close(this->_vacantFd));
}

bool mg::Acceptor::isListening()
{
    return this->_listen;
}

void mg::Acceptor::listen()
{
    this->_listen = true;
    this->_channel.enableReading();
    this->_socket.listen();
}

void mg::Acceptor::setNewConnectionCallBack(const NewConnectionCallBack callback)
{
    this->_callback = std::move(callback);
}

int mg::Acceptor::createNonBlockScoket(int domain, int type)
{
    this->_socket.setSocketType(domain, type);
    if (::fcntl(this->_socket.fd(), F_SETFL, ::fcntl(this->_socket.fd(), F_GETFL, 0) | O_NONBLOCK | O_CLOEXEC) < 0)
        LOG_ERROR("EventLoop[{}] create nonblocksocket failed", this->_loop->getLoopName());
    return this->_socket.fd();
}

void mg::Acceptor::handleReadEvent()
{
    InternetAddress address;
    int acceptFd = this->_socket.accept(&address);
    if (acceptFd >= 0)
    {
        if (_callback)
            _callback(acceptFd, address);
        else
        {
            LOG_ERROR("EventLoop[{}] _callback not set", this->_loop->getLoopName());
            ::close(acceptFd);
        }
    }
    else
    {
        LOG_ERROR("EventLoop[{}] accept failed", this->_loop->getLoopName());
        if (errno == EMFILE)
        {
            LOG_ERROR("EventLoop[{}] no vacant fileDescripter accept new connection", this->_loop->getLoopName());
            ::close(this->_vacantFd);
            TEMP_FAILURE_RETRY(this->_vacantFd = ::accept4(this->_socket.fd(), nullptr, nullptr, 0));
            ::close(this->_vacantFd);
            this->_vacantFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
