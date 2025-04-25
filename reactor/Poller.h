
#include "EventLoop.h"
#include <map>
#include <vector>
struct pollfd;
namespace mylib
{
    class Channel;
    class Poller
    {
    public:
        using ChannelList = std::vector<Channel *>;
        Poller(EventLoop *loop);
        ~Poller();

        Timestamp poll(int timeoutMs, ChannelList *activeChannels);
        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        void assertInLoopThread()
        {
            ownerLoop_->assertInLoopThread();
        }

    private:
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

        using PollFdList = std::vector<struct pollfd>;
        using ChannelMap = std::map<int, Channel *>;

        EventLoop *ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;
    };
};