#include "EventLoop.h"
void threadFUnc()
{
    EventLoop loop;
    loop.loop();
}

int main()
{
    EventLoop loop;

    thread a(threadFUnc);
    loop.loop();
}