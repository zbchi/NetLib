#pragma once
#include <string>
#include <netinet/in.h>
namespace mylib
{
    class InetAddress
    {
    private:
        struct sockaddr_in addr_;

    public:
        explicit InetAddress(uint16_t port);
        InetAddress(const std::string &ip, uint16_t port);
        InetAddress(const struct sockaddr_in &addr) : addr_(addr)
        {
        }

        const struct sockaddr_in &getSockAddrInet() const { return addr_; }
        void setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }

        std::string toHostPort() const;
    };
};