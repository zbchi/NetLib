#include "EventLoopThread.h"
using namespace mylib;
void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();
}

EventLoop *EventLoopThread::startLoop()
{
    thread_ = std::thread([this]()
                          { threadFunc(); });
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]
                   { return loop_ != nullptr; });
    }
    return loop_;
}
EventLoopThread::EventLoopThread()
    : loop_(nullptr),
      exiting_(false)
{
}
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    loop_->quit();
    thread_.join();
}