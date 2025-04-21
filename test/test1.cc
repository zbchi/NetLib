#include "EventLoop.h"
void threadFUnc()
{
    EventLoop loop;
    loop.loop();
}

int main()
{
    EventLoop loop;

    std::thread a(threadFUnc);
    loop.loop();
    a.join();
}