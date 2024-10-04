#include "acceptor.h"
#include "socket.h"
#include "log.h"

#include <fcntl.h>

mg::Acceptor::Acceptor(int domain, int type, EventLoop *loop, const InternetAddress &listenAddress, bool reusePort)
    : _loop(loop), _socket(0), _channel(loop, createNonBlockScoket(domain, type)),
      _vacantFd(::open("/dev/null", O_RDWR | O_CLOEXEC))
{
    _socket.setReuseAddress(true);
    _socket.bind(listenAddress);
}

mg::Acceptor::~Acceptor()
{
    this->_channel.disableAllEvents();
    this->_channel.remove();
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
        LOG_ERROR("create nonblocksocket failed");
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
            LOG_ERROR("_callback not set");
            ::close(acceptFd);
        }
    }
    else
    {
        LOG_ERROR("accept failed");
        if (errno == EMFILE)
        {
            LOG_ERROR("no vacant fileDescripter accept new connection");
            ::close(this->_vacantFd);
            TEMP_FAILURE_RETRY(this->_vacantFd = ::accept4(this->_socket.fd(), nullptr, nullptr, 0));
            ::close(this->_vacantFd);
            this->_vacantFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
