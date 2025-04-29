#include "EventLoop.h"
#include <cassert>
#include <cstdlib>
#include <unistd.h>
#include <sys/eventfd.h>
#include <signal.h>
#include "Epoller.h"
#include "Channel.h"
const int kPollTimeMs = 10000;

class IngnoreSigPipe
{
public:
    IngnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
IngnoreSigPipe initObj;

using namespace mylib;
__thread EventLoop *t_loopInThisThread = 0;

static int createEventfd()
{
    int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventFd < 0)
    {
        LOG_ERROR("Failed in eventfd");
        abort();
    }
    return eventFd;
}

EventLoop::EventLoop()
    : looping_(false),
      threadId_(CurrentThread::tid()),
      poller_(new Epoller(this)),
      quit_(false),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
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

    wakeupChannel_->setReadCallback([this](Timestamp)
                                    { handleRead(); });
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    ::close(wakeupFd_);
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
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin();
             it != activeChannels_.end(); it++)
        {
            (*it)->handleEvent(pollReturnTime_);
        }
        // LOG_TRACE("EventLoop %p stop looping", this);
        doPendingFunctors();
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
    if (!isInLoopThread())
        wakeup();
}
void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
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

void EventLoop::runInLoop(const Functor &cb)
{
    if (isInLoopThread())
        cb();
    else
        queueInLoop(cb);
}

void EventLoop::queueInLoop(const Functor &cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
        LOG_ERROR("EventLoop::wakeup() writes %ld bytes instead of 8", n);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
        LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8", n);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (size_t i = 0; i < functors.size(); i++)
    {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}