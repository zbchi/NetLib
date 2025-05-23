#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <stdio.h>
#include <unistd.h>
#include <thread>
using namespace mylib;
void newConnection(int sockfd, const InetAddress &peerAddr)
{
    printf("newConnection(): accepted a new connection from %s\n",
           peerAddr.toHostPort().c_str());
    ::write(sockfd, "How are you?\n", 13);
    sockets::close(sockfd);
}

int main()
{

    std::thread t1([]()
                   { InetAddress listenAddr(8080); 
                    EventLoop loop;
                    Acceptor acceptor(&loop,listenAddr);
                    acceptor.setNewConnectionCallback(newConnection);
                    acceptor.listen();
                    loop.loop(); });

    printf("main(): pid = %d\n", getpid());

    InetAddress listenAddr(9981);
    EventLoop loop;

    Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    loop.loop();
}