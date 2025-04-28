
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "SocketsOps.h"

#include <unistd.h>
using namespace mylib;

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &name,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop),
      name_(name),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      state_(kConnecting)
{
    LOG_DEBUG("TcpConnection::ctor[%s] at %p fd=%d",
              name_.c_str(), this, sockfd);
    channel_->setReadCallback([this](Timestamp receiveTime)
                              { handleRead(receiveTime); });
    channel_->setWriteCallback([this]()
                               { handleWrite(); });
    channel_->setCloseCallback([this]()
                               { handleClose(); });
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead(Timestamp receiveTime)

{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    else if (n == 0)
        handleClose();
    else
    {
        errno = savedErrno;
        LOG_SYSERR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        ssize_t n = ::write(channel_->fd(),
                            outputBuffer_.peek(),
                            outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    auto self = shared_from_this();
                    loop_->queueInLoop([self]()
                                       { self->writeCompleteCallback_(self); });
                }
                if (state_ = kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
            else
            {
                LOG_TRACE("I am going to write more data");
            }
        }
        else
        {
            LOG_SYSERR("TcpConnection::handleWrite");
        }
    }
    else
    {
        LOG_TRACE("Connection is down,no more writing");
    }
}

void TcpConnection::handleClose()
{
    LOG_TRACE("TcpConnection::handleClose state = %d", state_);
    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR("TcpConnection::handleError [%s] - SO_ERROR = %d %s",
              name_, err, strerror(err));
}
void TcpConnection::send(const std::string &message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            loop_->runInLoop([this, message]()
                             { sendInLoop(message); });
        }
    }
}

void TcpConnection::sendInLoop(const std::string &message)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote >= 0)
        {
            if (static_cast<size_t>(nwrote) < message.size())
            {
                LOG_TRACE("I am going to write more data");
            }
            else if (writeCompleteCallback_)
            {
                auto self = shared_from_this();
                loop_->queueInLoop([self]()
                                   { self->writeCompleteCallback_(self); });
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR("TcpConnection::sendInLoop");
            }
        }
    }

    if (static_cast<size_t>(nwrote) < message.size())
    {
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop([this]()
                         { shutdownInLoop(); });
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectDestroyed()
{
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());
    loop_->removeChannel(channel_.get());
}

void TcpConnection::setTcpNodelay(bool on)
{
    socket_->setTcpNodelay(on);
}
