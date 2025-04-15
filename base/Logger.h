#ifndef LOGGER_H
#define LOGGER_H
#include <cstring>
namespace mylib
{

    class Logger
    {
    public:
        enum LogLevel
        {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVALS,
        };
        class SourceFile
        {
        public:
            const char *data_;
            int size_;

            template <int N>
            SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1)
            {
                const char *slash = strrchr(data_, '/');
                if (slash)
                {
                    data_ = slash + 1;
                    size_ -= static_cast<int> data_ - arr;
                }
            }
            explicit SourceFile(const char *filename) : data_(filename)
            {
                const char *slash = strrchr(filename, '/');
                if (slash)
                {
                    data_ = slash + 1;
                }
                size_ = static_cast<int>(strlen(data_));
            }
        };

        static void log(SourceFile file, int line, LogLevel level, const char *fmt, ...);
        static void log(SourceFile file, int line, LogLevel level, const char *func, const char *fmt...);
        static void logSys(SourceFile file, int line, bool fatal, const char *fmt, ...);

        static LogLevel logLevel();
        static void setLogLevel(LogLevel level);
        typedef void (*OutputFunc)(const char *msg, int len);
        typedef void (*FlushFunc)();
        static void setOutput(OutputFunc);
        static void setFlush(FlushFunc);

    private:
        Logger() = delete;
    };

    extern Logger::LogLevel g_logLevel;

#define LOG_TRACE(fmt, ...)                  \
    if (Logger::logLevel() <= Logger::TRACE) \
    Logger::log(Logger::SourceFile(__FILE__), __LINE__, Logger::TRACE, __func__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)                  \
    if (Logger::LogLevel() <= Logger::DEBUG) \
    Logger::log(Logger::SourceFile(__FILE__), __LINE__, Logger::DEBUG, __func__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
    Logger::log(Logger::SourceFile(__FILE__), __LINE__, Logger::INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
    Logger::log(Logger::SourceFile(__FILE__), __LINE__, Logger::WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) \
    Logger::log(Logger::SourceFile(__FILE__), __LINE__, Logger::ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) \
    Logger::log(Logger::SourceFile(__FILE__), __LINE__, Logger::FATAL, fmt, ##__VA_ARGS__)
#define LOG_SYSERR(fmt, ...) \
    Logger::logSys(Logger::SourceFile(__FILE__), __LINE__, false, fmt, ##__VA_ARGS__)
#define LOG_SYSFATAL(fmt, ...) \
    Logger::logSys(Logger::SourceFile(__FILE__), __LINE__, true, fmt, ##__VA_ARGS__)
};

#endif