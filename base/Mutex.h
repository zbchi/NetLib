#include <pthread.h>
class MutexLock
{
private:
    pthread_mutex_t mutex_;
    pid_t holder_;

public:
    MutexLock() : holder_(0)
    {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~MutexLock()
    {
        assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    }

    bool isLockedByThisThread()
    {
        return holder_ == CurrentThread::tid();
    }

    void assertLocked()
    {
        assert(isLockedByThisThread());
    }

    void lock()
    {
        pthread_mutex_lock(&mutex_);
        holder_ = CurrentThread::tid();
    }

    void unlock()
    {
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t *getPthreadMutex()
    {
        return &mutex_;
    }
};

class MutexLockGuard
{
public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex)
    {
        mutex_.lock();
    }

    ~MutexLockGuard()
    {
        mutex_.unlock();
    }

private:
    MutexLock &mutex_;
};

#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name");

class Condition
{
private:
    MutexLock mutex_;
    pthread_cond_t pcond_;

public:
    explicit Condition(MutexLock &mutex) : mutex_(mutex)
    {
        pthread_cond_init(&pcond_, NULL);
    }

    ~Condition()
    {
        pthread_cond_destroy(&pcond_);
    }
    void wait()
    {
        pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
    }
    void notify()
    {
        pthread_cond_signal(&pcond_);
    }
    void notifyall()
    {
        pthread_cond_broadcast(&pcond_);
    }
};

class CountDownLatch
{
private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;

public:
    CountDownLatch(int count) : mutex_(), condition_(mutex_), count_(count)
    {
    }
};