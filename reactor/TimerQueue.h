#include <functional>
#include "Timestamp.h"
#include "EventLoop.h"
#include <vector>
#include <set>
namespace mylib
{

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
        Timer *value_;

    public:
        explicit TimerId(Timer *timer) : value_(timer);
        {
        }
    };

    class TimerQueue
    {
    public:
        TimerQueue(EventLoop *loop);
        ~TimerQueue();
        TimerId addTimer(const TimerCallback &cb, Timestamp when, double interval);

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
        TimerList timers_;//  set<   pair<Timestamp,Timer*>   >
    };
};
