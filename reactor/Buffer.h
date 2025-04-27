#include <vector>
#include <string>
namespace mylib
{
    class Buffer
    {
    private:
        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;

        char *begin() { return &*buffer_.begin(); }
        const char *begin() const { return &*buffer_.begin(); }
        void makeSpace(size_t len)
        {
            if (writableBytes() + prependableBytes() < len + kCheapPrepend)
            {
                buffer_.resize(writerIndex_ + len);
            }
            else
            {
                size_t readable = readableBytes();
                std::copy(begin() + readerIndex_,
                          begin() + writerIndex_,
                          begin() + kCheapPrepend);
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_ + readable;
            }
        }

    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;
        Buffer()
            : buffer_(kCheapPrepend + kInitialSize),
              readerIndex_(kCheapPrepend),
              writerIndex_(kCheapPrepend)
        {
        }
        ssize_t readFd(int fd, int *savedErrno);

        size_t readableBytes() const { return writerIndex_ - readerIndex_; }
        size_t writableBytes() const { return buffer_.size() - writerIndex_; }
        size_t prependableBytes() const { return readerIndex_; }
        const char *peek() const { return begin() + readerIndex_; }

        void retrieve(size_t len) { readerIndex_ += len; }
        void retrieveUntil(const char *end) { retrieve(end - peek()); }
        void retrieveAll()
        {
            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend;
        }
        std::string retrieveAsString()
        {
            std::string str(peek(), readableBytes());
            retrieveAll();
            return str;
        }

        void append(const char *data, size_t len)
        {
            ensureWritableBytes(len);
            std::copy(data, data + len, beginWrite());
            hasWritten(len);
        }

        void ensureWritableBytes(size_t len)
        {
            if (writableBytes() < len)
            {
                makeSpace(len);
            }
        }
        char *beginWrite() { return begin() + writerIndex_; }
        const char *beginWrite() const { return begin() + writerIndex_; }
        void hasWritten(size_t len) { writerIndex_ += len; }
    };
};