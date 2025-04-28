#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpClient.h"
#include <stdio.h>
#include <unistd.h>
std::string message = "Hello\n";
using namespace mylib;

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection():new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());
        conn->send(message);
    }
    else
    {
        printf("onConnection():connection [%s] is down",
               conn->name().c_str());
    }
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    printf("onMessage():received %zd bytes from conneciton [%s] ar %s\n",
           buf->readableBytes(),
           conn->name().c_str(),
           receiveTime.toFormattedString().c_str());
    printf("onMessage():[%s]\n", buf->retrieveAsString().c_str());
}

int main()
{
    EventLoop loop;
    InetAddress serverAddr("localhost", 9981);
    TcpClient client(&loop, serverAddr);

    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    client.connect();
    loop.loop();
}