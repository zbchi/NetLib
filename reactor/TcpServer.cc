#include "Logger.h"

#include "TcpServer.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
using namespace mylib;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
    : loop_(loop),
      name_(listenAddr.toHostPort()),
      acceptor_(new Acceptor(loop, listenAddr)),
      started_(false),
      nextConnId_(1)
{
    acceptor_->setNewConnectionCallback([this](int sockfd, const InetAddress &perrAddr)
                                        { this->newConnection(sockfd, perrAddr); });
}

TcpServer::~TcpServer() {}

void TcpServer::start()
{
    if (!started_)
        started_ = true;
    if (!acceptor_->isListenning())
    {
        loop_->runInLoop([this]()
                         { acceptor_->listen(); });
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s",
             name_.c_str(), connName.c_str(), peerAddr.toHostPort().c_str());

    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(
        new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback([this](const TcpConnectionPtr &conn)
                           { removeConnection(conn); });
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnection [%s] - connection %s",
             name_.c_str(), conn->name().c_str());
    connections_.erase(conn->name());
    loop_->runInLoop([conn]()
                     { conn->connectDestroyed(); });
}