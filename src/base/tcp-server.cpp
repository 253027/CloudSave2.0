#include "tcp-server.h"
#include "log.h"

mg::TcpServer::TcpServer(EventLoop *loop, const InternetAddress &listenAddress, const std::string &name, int domain, int type)
    : _isStarted(false), _name(name), _loop(loop),
      _acceptor(new Acceptor(domain, type, loop, listenAddress, true)),
      _connectionID(0),
      _threadInitialCallback(), _threadPool(new EventLoopThreadPool(loop, name)),
      _address(listenAddress)

{
    this->_acceptor->setNewConnectionCallBack(std::bind(&TcpServer::acceptorCallback, this, std::placeholders::_1, std::placeholders::_2));
}

mg::TcpServer::~TcpServer()
{
    for (auto &x : _connectionMemo)
    {
        TcpConnectionPointer connection(x.second);
        x.second.reset();
        connection->getLoop()->run(std::bind(&TcpConnection::connectionDestoryed, connection));
    }
}

void mg::TcpServer::setThreadInitialCallback(ThreadInitialCallBack callback)
{
    this->_threadInitialCallback = std::move(callback);
}

void mg::TcpServer::setWriteCompleteCallback(const WriteCompleteCallback &callback)
{
    this->_writeCompleteCallback = callback;
}

void mg::TcpServer::start()
{
    if (this->_isStarted)
        return;
    this->_isStarted = true;
    _threadPool->start(_threadInitialCallback);
    _loop->run(std::bind(&Acceptor::listen, this->_acceptor.get()));
}

const std::string &mg::TcpServer::getName() const
{
    return this->_name;
}

std::string mg::TcpServer::getIpPort()
{
    return _address.toIpPort();
}

std::string mg::TcpServer::getIp()
{
    return _address.toIp();
}

uint16_t mg::TcpServer::getPort()
{
    return _address.port();
}

void mg::TcpServer::setThreadNums(int nums)
{
    this->_threadPool->setThreadNums(nums);
}

void mg::TcpServer::setConnectionCallback(const TcpConnectionCallback &callback)
{
    this->_connectionCallback = callback;
}

void mg::TcpServer::setMessageCallback(const MessageDataCallback &callback)
{
    this->_messgageDataCallback = callback;
}

void mg::TcpServer::acceptorCallback(int fd, const InternetAddress &peerAddress)
{
    EventLoop *loop = this->_threadPool->getNextLoop();
    if (!loop)
    {
        LOG_ERROR("{} EventLoop nullptr", this->_name);
        return;
    }
    LOG_INFO("{} new connection:[{}] socketFd:{}", this->_name, peerAddress.toIpPort(), fd);

    char buf[1024] = {0};
    ::snprintf(buf, sizeof(buf), "-%s-%d", peerAddress.toIpPort().c_str(), ++this->_connectionID);
    std::string connectionName = this->_name + buf;

    // 拿到本机ip地址
    sockaddr_in local;
    ::memset(&local, 0, sizeof(local));
    socklen_t addresslen = sizeof(local);

    int ret = TEMP_FAILURE_RETRY(::getsockname(fd, (sockaddr *)&local, &addresslen));
    if (ret < 0)
    {
        TEMP_FAILURE_RETRY(::close(fd));
        LOG_ERROR("{} get local address failed", this->_name);
        return;
    }

    InternetAddress localAddress(local);
    TcpConnectionPointer connection = std::make_shared<TcpConnection>(loop, connectionName, fd, localAddress, peerAddress);
    connection->setConnectionCallback(this->_connectionCallback);
    connection->setMessageCallback(this->_messgageDataCallback);
    connection->setWriteCompleteCallback(this->_writeCompleteCallback);
    connection->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    this->_connectionMemo[connectionName] = connection;

    loop->run(std::bind(&TcpConnection::connectionEstablished, connection.get()));
}

void mg::TcpServer::removeConnection(const TcpConnectionPointer &connection)
{
    this->_loop->run(std::bind(&TcpServer::removeConnectionCallBack, this, connection));
}

void mg::TcpServer::removeConnectionCallBack(const TcpConnectionPointer &connection)
{
    this->_connectionMemo.erase(connection->name());
    EventLoop *loop = connection->getLoop();
    loop->push(std::bind(&TcpConnection::connectionDestoryed, connection));
    /*
     * Tips:
        这里有个注意事项loop->push中传入参数必须为一个智能指针而不是裸指针connection.get() !!!
        因为this->_connectionMemo中已经将所有权销毁了，即使运行到此函数共享指针计数不为0，
        但当eventloop执行doPendingFunctions时这个tcp连接的计数必定是0已经被析构掉了。
        此时再调用tcp连接的方法就会出现 "heap use after free" 错误
     */
}
