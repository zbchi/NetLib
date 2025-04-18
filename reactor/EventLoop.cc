#include "EventLoop.h"
#include <cassert>
#include <cstdlib>

#include "Poller.h"
#include "Channel.h"
#include "TimerQueue.h"

const int kPollTimeMs = 10000;

using namespace mylib;
__thread EventLoop *t_loopInThisThread = 0;
EventLoop::EventLoop()
    : looping_(false), threadId_(CurrentThread::tid()),
      poller_(new Poller(this)), quit_(false),
      timerQueue_(new TimerQueue(this))
{
    LOG_TRACE("EventLoop created %p in thread %d", this, threadId_);
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d",
                  t_loopInThisThread, threadId_);
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
    quit_ = false;
    while (!quit_)
    {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin();
             it != activeChannels_.end(); it++)
        {
            (*it)->handleEvent();
        }
        LOG_TRACE("EventLoop %p stop looping", this);
        looping_ = false;
    }
}
void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL("EventLoop::abortNotInLoopThread - EventLoop %p is accessed from thread %d (owner thread: %d)",
              this, mylib::CurrentThread::tid(), threadId_);
    abort();
}

void EventLoop::quit()
{
    quit_ = true;
}
void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb)
{
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}