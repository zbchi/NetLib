#pragma once
namespace mylib
{
    class InetAddress;
    class Socket
    {
    private:
        const int sockfd_;

    public:
        int fd() const { return sockfd_; };
        void bindAddress(const InetAddress &addr);
        void listen();
        int accept(InetAddress *peeraddr);

        void setTcpNodelay(bool on);
        void setReuseAddr(bool on);
        void shutdownWrite();
        explicit Socket(int sockfd) : sockfd_(sockfd) {}
        ~Socket();
    };
};
