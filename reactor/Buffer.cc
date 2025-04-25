#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>
using namespace mylib;
ssize_t Buffer::readFd(int fd, int *savedErrno)
{
    char extrabuf[65536];
    struct iovec[2];
    
}