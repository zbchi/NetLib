#include <thread>
#include "CurrentThread.h"
#include "Logger.h"
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
        return threadId_ == mylib::CurrentThread::tid();
    }
};