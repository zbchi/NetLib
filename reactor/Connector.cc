#include "Connector.h"
#include "EventLoop.h"
#include "Channel.h"
#include "SocketsOps.h"
#include <errno.h>
using namespace mylib;
// Connector.cc
const int mylib::Connector::kMaxRetryDelayMs;
const int mylib::Connector::kInitRetryDelayMs;
Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      retryDelayMs_(kInitRetryDelayMs)

{
}
Connector::~Connector() {}
void Connector::start()
{
    connect_ = true;
    loop_->runInLoop([this]()
                     { startInLoop(); });
}
void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    if (connect_)
    {
        connect();
    }
    else
    {
        LOG_DEBUG("do not connect");
    }
}

void Connector::connect()
{
    int sockfd = sockets::createNonblockingOrDie();
    int ret = sockets::connect(sockfd, serverAddr_.getSockAddrInet());
    int savedErrno = (ret == 0) ? 0 : errno;

    switch (savedErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_SYSERR("connect error in Connector::startInLoop %d", savedErrno);
        sockets::close(sockfd);
        break;

    default:
        LOG_SYSERR("Unexpected error in Connector::startInLoop %d", savedErrno);
        sockets::close(sockfd);
        break;
    }
}

void Connector::connecting(int sockfd)
{
    setState(kConnecting);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback([this]()
                               { handleWrite(); });
    channel_->setErrorCallback([this]()
                               { handleError(); });
    channel_->enableWriting();
}

void Connector::retry(int sockfd)
{
    sockets::close(sockfd);
    setState(kDisconnected);
    if (connect_)
    {
        LOG_INFO("Connector::retry - Retry connecting to %s in %d milliseconds.",
                 serverAddr_.toHostPort().c_str(), retryDelayMs_);
        timerId_ = loop_->runAfter(retryDelayMs_ / 1000.0,
                                   [this]()
                                   { startInLoop(); });
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_DEBUG("do not connect");
    }
}

void Connector::stop()
{
    connect_ = false;
    // loop->cancle(timerId_);
}

void Connector::handleWrite()
{
    LOG_TRACE("Connector::handleWrite %d", state_);
    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if (err)
        {
            LOG_WARN("Connector::handleWrite - SO_ERROR = %d %s",
                     err, strerror_tl(err));
            retry(sockfd);
        }
        else if (sockets::isSelfConnect(sockfd))
        {
            LOG_WARN("Connector::handleWrite - Self connect");
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if (connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                sockets::close(sockfd);
            }
        }
    }
}
void Connector::handleError()
{
    LOG_ERROR("Connect::handleError");
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    LOG_TRACE("SO_ERROR = %d %s", err, strerror_tl(err));
    retry(sockfd);
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    loop_->removeChannel(channel_.get());
    int sockfd = channel_->fd();
    loop_->queueInLoop([this]()
                       { this->channel_.reset(); });
    return sockfd;
}