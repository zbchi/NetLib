#include "Epoller.h"
#include "Channel.h"
#include <errno.h>
#include <unistd.h>
using namespace mylib;

namespace
{
    const int kNew = -1;
    const int kAdded = 1;
    const int kDelete = 2;
}

Epoller::Epoller(EventLoop *loop)
    : ownerLoop_(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("Epoller::Epoller");
    }
}

Epoller::~Epoller()
{
    ::close(epollfd_);
}

Timestamp Epoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    Timestamp now(Timestamp ::now());
    if (numEvents > 0)
    {
        LOG_TRACE("%d events happended", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_TRACE("nothing happended");
    }
    else
    {
        LOG_SYSERR("Epoller::poll()");
    }
    return now;
}

void Epoller::fillActiveChannels(int numEvents,
                                 ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void Epoller::updateChannel(Channel *channel)
{
    assertInLoopThread();
    LOG_TRACE("fd=%d events=", channel->fd(), channel->events());
    const int index = channel->index();
    if (index == kNew || index == kDelete)
    {
        int fd = channel->fd();
        if (index == kNew)
        {
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDelete);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Epoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_SYSERR("epoll_ctl op=%d fd=%d", operation, fd);
        }
        else
        {
            LOG_SYSFATAL("epoll_ctl op=%d fd=%d", operation, fd);
        }
    }
}

void Epoller::removeChannel(Channel *channel)
{
    assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE("fd=%d", fd);
    int index = channel->index();
    channels_.erase(fd);
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}