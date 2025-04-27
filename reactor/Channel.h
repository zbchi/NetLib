
#pragma once
#include <functional>
namespace mylib
{
    class EventLoop;
    class Timestamp;
    class Channel
    {

    public:
        using ReadEventCallBack = std::function<void(Timestamp)>;
        using EventCallBack = std::function<void()>;
        Channel(EventLoop *loop, int fdArg);
        ~Channel();
        void handleEvent(Timestamp receiveTime);
        void setReadCallback(const ReadEventCallBack &cb) { readCallback_ = cb; }
        void setWriteCallback(const EventCallBack &cb) { writeCallback_ = cb; }
        void setErrorCallback(const EventCallBack &cb) { errorCallback_ = cb; }

        int fd() const { return fd_; }
        int events() const { return events_; }
        bool isNoneEvent() const { return events_ == kNoneEvent; }

        void enableReading()
        {
            events_ |= kReadEvent;
            update();
        }
        void disableAll()
        {
            events_ = kNoneEvent;
            update();
        }

        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }
        void set_revents(int event) { revents_ = event; }
        EventLoop *ownerLoop() { return loop_; }

    private:
        void update();

        ReadEventCallBack readCallback_;
        EventCallBack writeCallback_;
        EventCallBack errorCallback_;
        EventCallBack closeCallback_;

        EventLoop *loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_;
        bool eventHandling_;

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;
    };
};