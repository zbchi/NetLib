#include "SocketsOps.h"
#include "Logger.h"
#include <unistd.h>
#include <cstdio>
using namespace mylib;

using SA = struct sockaddr;
const SA *sockaddr_cast(const struct sockaddr_in *addr)
{
    return static_cast<const SA *>(static_cast<const void *>(addr));
}

SA *sockaddr_cast(struct sockaddr_in *addr)
{
    return static_cast<SA *>(static_cast<void *>(addr));
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in &addr)
{
    if (::bind(sockfd, sockaddr_cast(&addr), sizeof addr) < 0)
    {
        LOG_SYSFATAL("sockets::bindOrDie");
    }
}
void sockets::listenOrDie(int sockfd)
{
    if (::listen(sockfd, SOMAXCONN) < 0)
    {
        LOG_FATAL("sockets::listenOrDie");
    }
}
int sockets::accept(int sockfd, struct sockaddr_in *addr)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        LOG_SYSERR("Sockets::accept");
    }
    return connfd;
}

int sockets::createNonblockingOrDie()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL("sockets::createNonblockingOrDie");
    }
    return sockfd;
}

void sockets::close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        LOG_SYSERR("sockets::close");
    }
}

void sockets::fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOG_SYSERR("sockets::fromHostPort");
    }
}

void sockets::toHostPort(char *buf, size_t size, const struct sockaddr_in &addr)
{
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
    uint16_t port = sockets::networkToHost16(addr.sin_port);
    snprintf(buf, size, "%s:%u", host, port);
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in localAddr;
    socklen_t addrlen = sizeof localAddr;
    if (::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen) < 0)
    {
        LOG_SYSERR("sockets::getLocalAddr");
    }
    return localAddr;
}