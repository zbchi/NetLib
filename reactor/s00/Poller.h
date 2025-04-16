
#include "EventLoop.h"
struct pollfd;
namespace mylib
{
    class Channel;
    class Poller
    {
    public:
        using std::vector<Channel *> ChannelList;
        Poller(EventLoop *loop);
        ~Poller();

        Timestamp poll(int timeoutMs, ChannelList *activeChannels);
        void updateChannel(Channel *channel);
        void assertInLoopThread()
        {
            ownerLoop_->assertInLoopThread();
        }

    private:
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
        
        using std::vector<struct pollfd> PollFdList;
        using std::map<int, Channel *> ChannelMap;

        EventLoop *ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;
    };
}