#pragma once
#include <functional>
#include "Timestamp.h"

#include "Channel.h"
#include <vector>
#include <set>
namespace mylib
{

    using TimerCallback = std::function<void()>;
    class Timer
    {
    private:
        const TimerCallback callback_;
        Timestamp expiration_;
        const double interval_;
        const bool repeat_;

    public:
        Timer(const TimerCallback &cb, Timestamp when, double interval)
            : callback_(cb), expiration_(when), interval_(interval), repeat_(interval_ > 0) {}

        void run() const { callback_(); };

        Timestamp expiration() const { return expiration_; };
        bool repeat() const { return repeat_; };
        void restart(Timestamp now);
    };

    class TimerId
    {
    private:
        Timer *timer_;
        int64_t sequence_;

    public:
        explicit TimerId(Timer *timer = NULL, int64_t seq = 0)
            : timer_(timer),
              sequence_(seq)
        {
        }
        friend class TimerQueue;
    };

    class TimerQueue
    {
    public:
        TimerQueue(EventLoop *loop);
        ~TimerQueue();
        TimerId addTimer(const TimerCallback &cb, Timestamp when, double interval);
        void addTimerInLoop(Timer *timer);

    private:
        using Entry = std::pair<Timestamp, Timer *>;
        using TimerList = std::set<Entry>;

        void handleRead();

        std::vector<Entry> getExpired(Timestamp now);
        void reset(const std::vector<Entry> &expired, Timestamp now);
        bool insert(Timer *timer);

        EventLoop *loop_;
        const int timerfd_;
        Channel timerfdChannel_;
        TimerList timers_; //  set<   pair<Timestamp,Timer*>   >
    };
};
