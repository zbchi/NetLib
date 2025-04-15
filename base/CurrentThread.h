#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H
#include <sys/types.h>
namespace mylib
{
    namespace CurrentThread
    {
        extern __thread pid_t t_cachedTid;
        pid_t tid();
        pid_t gettid();

    }
}

#endif