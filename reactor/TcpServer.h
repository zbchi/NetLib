#include "TcpConnection.h"

#include <string>
#include <memory>
#include <map>
namespace mylib
{
    class EventLoop;
    class InetAddress;
    class Acceptor;
    class EventLoopThreadPool;
    class TcpServer
    {
    public:
        TcpServer(EventLoop *loop, const InetAddress &listenAddr);
        ~TcpServer();
        void start();

        void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
        void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
        void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

        void removeConnection(const TcpConnectionPtr &conn);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

        void setThreadNum(int numThreads);

    private:
        using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
        void newConnection(int sockfd, const InetAddress &peerAddr);

        EventLoop *loop_;
        int nextConnId_;
        bool started_;
        const std::string name_;

        std::unique_ptr<Acceptor> acceptor_;
        std::unique_ptr<EventLoopThreadPool> threadPool_;
        ConnectionMap connections_;

        MessageCallback messageCallback_;
        ConnectionCallback connectionCallback_;
        WriteCompleteCallback writeCompleteCallback_;
    };

};