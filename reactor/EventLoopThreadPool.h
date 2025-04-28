#include <vector>
#include "EventLoopThread.h"
namespace mylib
{
    class EventLoopThreadPool
    {
    private:
        EventLoop *baseLoop_;
        bool started_;
        int numThreads_;
        int next_;
        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop *> loops_;

    public:
        EventLoopThreadPool(EventLoop *baseLoop);
        ~EventLoopThreadPool();
        void setThreadNum(int numThreads) { numThreads_ = numThreads; }
        void start();
        EventLoop *getNextLoop();
    };

}