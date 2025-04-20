
#include "Channel.h"
#include "Socket.h"
#include <functional>
namespace mylib
{
    class EventLoop;
    class InetAddress;
    class Acceptor
    {
    public:
        using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;

        Acceptor(EventLoop *loop, const InetAddress &listenAddr);
        ~Acceptor();

        void listen();
        void setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; }

    private:
        EventLoop *loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        bool listenning_;
        NewConnectionCallback newConnectionCallback_;

        void handleRead();
    };
};