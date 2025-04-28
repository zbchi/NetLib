#include "TcpConnection.h"
#include <mutex>
namespace mylib
{
    class Connector;
    using ConnectorPtr = std::shared_ptr<Connector>;

    class TcpClient
    {
    private:
        void newConnection(int sockfd);
        void removeConnection(const TcpConnectionPtr &conn);

        bool connect_;
        bool retry_;
        int nextConnId_;
        EventLoop *loop_;
        ConnectorPtr connector_;
        ConnectionCallback connecitonCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;

        TcpConnectionPtr connection_;
        std::mutex mutex_;

    public:
        TcpClient(EventLoop *loop, const InetAddress &serverAddr);
        ~TcpClient();
        void setConnectionCallback(const ConnectionCallback &cb) { connecitonCallback_ = cb; }
        void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
        void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

        void connect();
        void disconnect();
        void stop();
    };
}