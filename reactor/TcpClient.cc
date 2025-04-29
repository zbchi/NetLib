#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include <string>
using namespace mylib;
TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),
      connector_(new Connector(loop, serverAddr)),
      retry_(false),
      connect_(true),
      nextConnId_(1)
{
    connector_->setNewConnectionCallback([this](int sockfd)
                                         { newConnection(sockfd); });
    LOG_INFO("TcpClient::TcpClient[%p] - connector %p", this, connector_.get());
}
TcpClient::~TcpClient() {}
void TcpClient::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = buf;

    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(loop_, connName,
                                            sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(connecitonCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback([this](const TcpConnectionPtr &conn)
                           { removeConnection(conn); });
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connection_.reset();
    }
    loop_->queueInLoop([conn]()
                       { conn->connectDestroyed(); });
    if (retry_ && connect_)
    {
        LOG_INFO("TcpClient::connect[%p] - Reconnecting to %s",
                 this, connector_->serverAddress().toHostPort().c_str());
        connector_->restart();
    }
}

void TcpClient::connect()
{
    LOG_INFO("TcpClient::connect[%p] - connecting to %s",
             this, connector_->serverAddress().toHostPort().c_str());
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (connection_)
        {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}