#include <functional>
class EventLoop;
class Channel
{

public:
    typedef function<void> EventCallBack;
    Channel(EventLoop *loop, int fd);

    void handleEvent();
    void setReadCallback(const EventCallBack &cb)
    {
        readCallback_ = cb;
    }
    void setWriteCallback(const EventCallBack &cb)
    {
        writeCallback_ = cb;
    }
    void setErrorCallback(const EventCallback &cb)
    {
        errorCallback_ = cb;
    }

    int fd() const
    {
        return fd_;
    }
    int events() const
    {
        return events_;
    }
    bool isNoneEvent() const
    {
        return events_ = kNoneEvent;
    }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }

    int index()
    {
        return index_;
    }
    void set_index(int idx)
    {
        index = idx;
    }
    EventLoop *ownerLoop()
    {
        return loop_;
    }

private:
    void update();

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;

    EventLoop *loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
}