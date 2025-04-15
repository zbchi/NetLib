#include <thread>
#include "CurrentThread.h"
#include
class EventLoop
{
private:
    void abortNotInLoopThread();

    bool looping_;
    const pid_t threadId_;

public:
    EventLoop();
    ~EventLoop();
    void loop();
    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const
    {
        return threadId_ == CurrentThread::tid();
    }
};