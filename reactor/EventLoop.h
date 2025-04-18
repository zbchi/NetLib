#pragma once
#include <thread>
#include "CurrentThread.h"
#include "Logger.h"
#include <vector>
namespace mylib
{
    using TimerCallback = std::function<void()>;
    class Poller;
    class Channel;
    class TimerQueue;
    class Timestamp;
    class TimerId;
    class EventLoop
    {
    private:
        void abortNotInLoopThread();

        bool looping_;
        const pid_t threadId_;

        using ChannelList = std::vector<Channel *>;
        bool quit_;

        std::unique_ptr<Poller> poller_;
        std::unique_ptr<TimerQueue> timerQueue_;
        ChannelList activeChannels_;

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

        bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
        void quit();
        void updateChannel(Channel *channel);

        TimerId runAt(const Timestamp &time, const TimerCallback &cb);
        TimerId runAfter(double delay, const TimerCallback &cb);
        TimerId runEvery(double interval, const TimerCallback &cb);
    };
};