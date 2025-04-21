
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Socket.h"
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
    channel_->setReadCallback([this]()
                              { handleRead(); });
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead()
{
    char buf[65536];
    ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
    if (n > 0)
        messageCallback_(shared_from_this(), buf, n);
    else if (n == 0)
        handleClose();
    else
        handleError();
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

void TcpConnection::connectDestroyed()
{
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());
    loop_->removeChannel(channel_.get());
}