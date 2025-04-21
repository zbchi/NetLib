#include "InetAddress.h"

#include <memory>
#include <functional>
namespace mylib
{
    class TcpConnection;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr &,
                                               const char *data,
                                               ssize_t len)>;
    class EventLoop;
    class Socket;
    class Channel;
    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {

    public:
        TcpConnection(EventLoop *loop,
                      const std::string &name,
                      int sockfd,
                      const InetAddress &localAddr,
                      const InetAddress &peerAddr);

        bool connected() { return state_ == kConnected; }
        const std::string &name() const { return name_; }
        const InetAddress &localAddress() { return localAddr_; }
        const InetAddress &peerAddress() { return peerAddr_; }

        void connectEstablished();

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            connectionCallback_ = cb;
        }
        void setMessageCallback(const MessageCallback &cb)
        {
            messageCallback_ = cb;
        }

    private:
        enum StateE
        {
            kConnecting,
            kConnected,
        };
        void setState(StateE s) { state_ = s; }

        void handleRead();

        EventLoop *loop_;
        std::string name_;
        StateE state_;

        InetAddress localAddr_;
        InetAddress peerAddr_;

        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;

        MessageCallback messageCallback_;
        ConnectionCallback connectionCallback_;
    };
};