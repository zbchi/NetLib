#include "CurrentThread.h"
#include <sys/syscall.h>
#include <unistd.h>
namespace mylib
{
    namespace CurrentThread
    {
        inline pid_t tid()
        {
            if (t_cachedTid == 0)
            {
                t_cachedTid = gettid();
            }
            return t_cachedTid;
        }
        pid_t gettid()
        {
            return static<pid_t>(::syscall(SYS_gettid));
        }
    }
}