#include "EventLoop.h"
#include <thread>
#include <condition_variable>
namespace mylib
{
    class EventLoopThread
    {
    public:
        EventLoopThread();
        ~EventLoopThread();
        EventLoop *startLoop();

    private:
        void threadFunc();

        EventLoop *loop_;
        bool exiting_;
        std::thread thread_;
        std::mutex mutex_;
        std::condition_variable cond_;
    };
}