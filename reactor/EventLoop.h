#pragma once
#include <thread>
#include "CurrentThread.h"
#include "Logger.h"
#include "Timestamp.h"
#include "TimerQueue.h"
#include <vector>
#include <functional>
#include <mutex>
namespace mylib
{
    using TimerCallback = std::function<void()>;
    class Epoller;
    class Channel;
    class TimerQueue;
    class Timestamp;
    class EventLoop
    {
    private:
        void abortNotInLoopThread();

        bool looping_;
        const pid_t threadId_;
        Timestamp pollReturnTime_;

        using ChannelList = std::vector<Channel *>;
        using Functor = std::function<void()>;
        bool quit_;

        std::unique_ptr<Epoller> poller_;
        std::unique_ptr<TimerQueue> timerQueue_;
        ChannelList activeChannels_;

        void handleRead();
        void doPendingFunctors();

        bool callingPendingFunctors_;
        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;
        std::mutex mutex_;
        std::vector<Functor> pendingFunctors_;

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
        void removeChannel(Channel *channel);

        TimerId runAt(const Timestamp &time, const TimerCallback &cb);
        TimerId runAfter(double delay, const TimerCallback &cb);
        TimerId runEvery(double interval, const TimerCallback &cb);

        void runInLoop(const Functor &cb);
        void queueInLoop(const Functor &cb);
        void wakeup();
    };
};