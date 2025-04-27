#include "Acceptor.h"
#include "Socket.h"
#include "Channel.h"
#include "SocketsOps.h"
#include "InetAddress.h"
#include "EventLoop.h"
using namespace mylib;
Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()),
      acceptChannel_(loop_, acceptSocket_.fd()),
      listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback([this](Timestamp)
                                   { handleRead(); });
}
Acceptor::~Acceptor() {}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            sockets::close(connfd);
        }
    }
}
