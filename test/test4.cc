#include "EventLoop.h"
#include <stdio.h>
int cnt = 0;
mylib::EventLoop *g_loop;

void print(const char *msg)
{
    printf("msg %s %s\n", mylib::Timestamp::now().toString().c_str(), msg);
}

int main()
{
    mylib::EventLoop loop;
    g_loop = &loop;

    loop.runAfter(1, []()
                  { print("once1"); });
    loop.runAfter(1.5, []()
                  { print("once1.5"); });
    loop.runAfter(3, []()
                  { print("once3"); });

    loop.runEvery(2, []()
                  { print("every2"); });
    loop.runEvery(3, []()
                  { print("every3"); });

    loop.loop();
}