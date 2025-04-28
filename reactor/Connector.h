#include <functional>
#include <memory>
#include "InetAddress.h"
#include "TimerQueue.h"
namespace mylib
{
    class Channel;
    class EventLoop;
    class Connector
    {
    public:
        using NewConnectionCallback = std::function<void(int sockfd)>;
        Connector(EventLoop *loop, const InetAddress &serverAddr);
        ~Connector();
        void setNewConnectionCallback(const NewConnectionCallback &cb)
        {
            newConnectionCallback_ = cb;
        }
        void start();
        void restart();
        void stop();
        const InetAddress &serverAddress() const { return serverAddr_; }

    private:
        enum States
        {
            kDisconnected,
            kConnecting,
            kConnected
        };
        static const int kMaxRetryDelayMs = 30 * 1000;
        static const int kInitRetryDelayMs = 500;
        int retryDelayMs_;
        void setState(States s) { state_ = s; }

        bool connect_;
        States state_;
        EventLoop *loop_;
        std::unique_ptr<Channel> channel_;
        InetAddress serverAddr_;

        NewConnectionCallback newConnectionCallback_;
        TimerId timerId_;
        void startInLoop();
        void connect();
        void connecting(int sockfd);
        void handleWrite();
        void handleError();
        void retry(int sockfd);
        int removeAndResetChannel();
    };
    using ConnectorPtr = std::shared_ptr<Connector>;
}