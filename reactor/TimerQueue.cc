#include "TimerQueue.h"
#include "Channel.h"
#include "Logger.h"

#include <sys/timerfd.h>
using namespace mylib;
void Timer::restart(Timestamp now)
{
    if (repeat_)
    {
        expiration_ = addTime(now, interval_);
    }
    else
    {
        expiration_ = Timestamp::invalid();
    }
}

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
        LOG_SYSFATAL("Failed in timerfd_create");
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int 64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
        microseconds = 100;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);p::kMicroSecondsPerSecond * 1000);
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue, oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);

    newValue.it_value = howMuchTimeFromNow(expiration);
    newValue.it_interval = expiration.invalid;
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_SYSERR("timerfd_settime()");
    }
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE("TimerQueue::handleRead() reads %zd at %s", n, now.toString().c_str());
    if (n != sizeof howmany)
        LOG_ERROR("TimeQueue::handleRead() reads %zd bytes instead of 8", n)
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop), timerfd_(createTimerfd()), timerfdChannel_(loop, timerfd_)
{
    timerfdChannel_.setReadCallback([this]()
                                    { handleRead(); });
    timerfdChannel_.enableReading();
}
TimerQueue::~TimerQueue()
{
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timerstamp now(Timestamp::now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);
    for (auto it = expired.begin(); it != expired.end(); it++)
    {
        it->second->run();
    }
    reset(expired, now);
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now)
{
    for (auto it = expired.begin(); it != expired.end(); it++)
    {
        if (it->second->repeat())
        {
            it->second->restart(now);
            insert(it->second);
        }
        else
        {
            delete it->second;
        }
    }

    if (!timers_.empty())
    {
        Timestamp nextExpire = timers_.begin()->first;
        resetTimerfd(timerfd_, nextExpire);
    }
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    auto it = timers_.lower_bound(sentry);
    assert(it == timers_end() || now < it->first);
    std::copy(timers_.begin(), it, std::back_inserter(expired));
    timers_.erase(timers_.begin(), it);
    return expired;
}

bool TimerQueue::insert(Timer *timer)
{
    Timestamp when = timer->expiration();
    timers_.insert(std::make_pair(when, timer));

    auto it = timers_.begin();
    bool earliestChanged = (it == timers_.end() || when < it->first);
    return earliestChanged;
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval)
{
    Timer *timer = new Timer(cb, when, interval);
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if (earliestChanged)
        resetTimerfd(timerfd_, when);
    return TimerId(timer);
}