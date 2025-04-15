#include "EventLoop.h"
#include "Logger.h"
__thread EventLoop *t_loopInThisThread = 0;
EventLoop::EventLoop() : looping_(false), threadId(CurrenttThread::tid())
{
    LOG_TRACE("EventLoop created %p in thread %%d", this, threadId_);
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    ::poll(NULL, 0, 5 * 1000);

    LOG_TRACE("EventLoop %p stop looping", this);
    looping_ = false;
}