#include "EventLoopThreadPool.h"
using namespace mylib;
EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(0),
      next_(0)
{
}
EventLoopThreadPool::~EventLoopThreadPool()
{
}
void EventLoopThreadPool::start()
{
    baseLoop_->assertInLoopThread();
    started_ = true;
    for (int i = 0; i < numThreads_; i++)
    {
        auto t = std::make_unique<EventLoopThread>();
        EventLoop *loop = t->startLoop();
        loops_.push_back(loop);
        threads_.push_back(std::move(t));
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    EventLoop *loop = baseLoop_;
    if (!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}