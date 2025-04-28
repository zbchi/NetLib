#include "Connector.h"
#include "EventLoop.h"
#include <stdio.h>
using namespace mylib;
EventLoop *g_loop;

void connectionCallback(int sockfd)
{
    printf("connected.\n");
    // g_loop->quit();
}
int main(int argc, char *argv[])
{
    EventLoop loop;
    g_loop = &loop;
    InetAddress addr("127.0.0.1", 9981);
    ConnectorPtr connector(new Connector(&loop, addr));
    connector->setNewConnectionCallback(connectionCallback);
    connector->start();
    loop.loop();
}