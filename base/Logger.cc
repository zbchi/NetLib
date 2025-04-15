#include "Logger.h"
#include <mutex>
#include <thread>
#include <cstdarg>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace mylib
{
    static std::mutex g_mutex;
    static Logger::OutputFunc g_output = [](const char *msg, int len)
    { fwrite(msg, 1, len, stdout); };
    static Logger::FlushFunc g_flush = []()
    { fflush(stdout); };
    Logger::LogLevel g_logLevel = Logger::TRACE;

    __thread char t_errnobuf[512];
    __thread char t_time[32];
    __thread time_t t_lastSecond;

    const char *strerror_tl(int savedErrno)
    {
        return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
    }

    static void formatTime(char *buf, size_t size)
    {
        time_t now = time(nullptr);
        if (now != t_lastSecond)
        {
            t_lastSecond = now;
            struct tm tm_time;
            gmtime_r(&now, &tm_time);
            snprintf(buf, size, "%4d%02d%02d %02d:%02d:%02d",
                     tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        }
    }

    static const char *levelNames[] = {
        "TRACE",
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
        "FATAL",
    };

    void Logger::log(SourceFile file, int line, LogLevel level, const char *fmt, ...)
    {
        if (level < g_logLevel)
            return;

        char time_buf[32];
        formatTime(time_buf, sizeof(time_buf));

        char msg_buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
        va_end(args);

        char line_buf[2048];
        snprintf(line_buf, sizeof(line_buf), "[%s] [%s] %s:%d: %s\n",
                 time_buf, levelNames[level], file.data_, line, msg_buf);

        std::lock_guard<std::mutex> lock(g_mutex);
        g_output(line_buf, strlen(line_buf));
        g_flush();
        if (level == FATAL)
            abort();
    }

    void Logger::log(SourceFile file, int line, LogLevel level, const char *func, const char *fmt, ...)
    {
        if (level < g_logLevel)
            return;

        char time_buf[32];
        formatTime(time_buf, sizeof(time_buf));

        char msg_buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
        va_end(args);

        char line_buf[2048];
        snprintf(line_buf, sizeof(line_buf), "[%s] [%s] %s %s:%d: %s\n",
                 time_buf, levelNames[level], func, file.data_, line, msg_buf);

        std::lock_guard<std::mutex> lock(g_mutex);
        g_output(line_buf, strlen(line_buf));
        g_flush();
        if (level == FATAL)
            abort();
    }

    void Logger::logSys(SourceFile file, int line, bool fatal, const char *fmt, ...)
    {
        int savedErrno = errno;
        LogLevel level = fatal ? FATAL : ERROR;
        if (level < g_logLevel)
            return;

        char time_buf[32];
        formatTime(time_buf, sizeof(time_buf));

        char msg_buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
        va_end(args);

        char line_buf[2048];
        snprintf(line_buf, sizeof(line_buf), "[%s] [%s] %s:%d: %s (errno=%d: %s)\n",
                 time_buf, levelNames[level], file.data_, line, msg_buf, savedErrno, strerror_tl(savedErrno));

        std::lock_guard<std::mutex> lock(g_mutex);
        g_output(line_buf, strlen(line_buf));
        g_flush();
        if (fatal)
            abort();
    }

    Logger::LogLevel Logger::logLevel()
    {
        return g_logLevel;
    }

    void Logger::setLogLevel(LogLevel level)
    {
        g_logLevel = level;
    }

    void Logger::setOutput(OutputFunc output)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_output = output;
    }

    void Logger::setFlush(FlushFunc flush)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_flush = flush;
    }

};