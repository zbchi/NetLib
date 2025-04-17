#include "CurrentThread.h"
#include <sys/syscall.h>
#include <unistd.h>
namespace mylib
{
    namespace CurrentThread
    {
        __thread pid_t t_cachedTid = 0;
        pid_t tid()
        {
            if (t_cachedTid == 0)
            {
                t_cachedTid = gettid();
            }
            return t_cachedTid;
        }
        pid_t gettid()
        {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}