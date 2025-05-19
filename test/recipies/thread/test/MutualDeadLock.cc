#include "myhpp.h"

class Request;
class Inventory
{
private:
    mutex mutex_;
    set<Request *> requests_;

public:
    void add(Request *req)
    {
        lock_guard<mutex> lock(mutex_);
        requests_.insert(req);
    }

    void remove(Request *req)
    {
        lock_guard<mutex> lock(mutex_);
        requests_.erase(req);
    }
    void printAll();
};

Inventory g_inventory;

class Request
{
private:
    mutex mutex_;

public:
    void process()
    {
        lock_guard<mutex> lock(mutex_);
        g_inventory.add(this);
    }

    ~Request()
    {
        lock_guard<mutex> lock(mutex_);
        sleep(1);
        g_inventory.remove(this);
    }
    void print()
    {
        lock_guard<mutex> lock(mutex_);
    }
};

void Inventory::printAll()
{
    lock_guard<mutex> lock(mutex_);
    sleep(1);
    for (auto req : requests_)
    {
        req->print();
    }
    printf("Inventory::printAll() unlocked\n");
}

void threadFunc()
{
    Request *req = new Request;
    req->process();
    delete req;
}

int main()
{
    thread thread(threadFunc); // 先加request锁，再加inventory锁
    usleep(500 * 1000);
    g_inventory.printAll(); // 先加inventory锁，再加request锁

    // deadlock
    thread.join();
}