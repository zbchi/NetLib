#include <map>
#include <vector>
#include <sys/epoll.h>
#include "EventLoop.h"
namespace mylib
{
    class Channel;
    class Epoller
    {
    public:
        using ChannelList = std::vector<Channel *>;
        Epoller(EventLoop *loop);
        ~Epoller();
        Timestamp poll(int timeoutMs, ChannelList *activeChannels);
        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

    private:
        static const int kInitEventListSize = 16;
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
        void update(int operation, Channel *channel);
        using EventList = std::vector<struct epoll_event>;
        using ChannelMap = std::map<int, Channel *>;

        EventLoop *ownerLoop_;
        int epollfd_;
        EventList events_;
        ChannelMap channels_;
    };
}