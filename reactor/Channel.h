#include <functional>
namespace mylib
{
    class EventLoop;
    class Channel
    {

    public:
        using EventCallBack = std::function<void()>;
        Channel(EventLoop *loop, int fdArg);

        void handleEvent();
        void setReadCallback(const EventCallBack &cb) { readCallback_ = cb; }
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

        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }
        void set_revents(int event) { revents_ = event; }
        EventLoop *ownerLoop() { return loop_; }

    private:
        void update();

        EventCallBack readCallback_;
        EventCallBack writeCallback_;
        EventCallBack errorCallback_;

        EventLoop *loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_;

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;
    };
};