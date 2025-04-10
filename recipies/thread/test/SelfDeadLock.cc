#include "myhpp.h"
class Request
{
private:
    mutex mutex_;

public:
    void process()
    {
        lock_guard<mutex> lock(mutex_);
        print();
    }

    void print()
    {
        lock_guard<mutex> lock(mutex_);
    }

    void WithLockHold()
    {
    }
};
int main()
{
    Request req;
    req.process();
}