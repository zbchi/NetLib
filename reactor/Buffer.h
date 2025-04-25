#include <vector>
namespace mylib
{
    class Buffer
    {
    private:
        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;

    public:
        ssize_t readFd(int fd, int *savedErrno);

        size_t readableBytes()const{return }
    };
};