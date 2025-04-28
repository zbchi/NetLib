#include "SocketsOps.h"
#include "Logger.h"
#include <unistd.h>
#include <cstdio>
#include <cerrno>
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

int sockets::connect(int sockfd, const struct sockaddr_in &addr)
{
    return ::connect(sockfd, sockaddr_cast(&addr), sizeof addr);
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

int sockets::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = sizeof optval;
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
        return optval;
}
struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = sizeof(peeraddr);
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
    {
        LOG_SYSERR("sockets::getPeerAddr");
    }
    return peeraddr;
}

void sockets::shutdownWrite(int sockfd)
{
    if (::shutdown(sockfd, SHUT_WR) < 0)
        LOG_SYSERR("sockets::shutdownWrite");
}

bool sockets::isSelfConnect(int sockfd)
{
    struct sockaddr_in localaddr = getLocalAddr(sockfd);
    struct sockaddr_in peeraddr = getPeerAddr(sockfd);
    return localaddr.sin_port == peeraddr.sin_port && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}