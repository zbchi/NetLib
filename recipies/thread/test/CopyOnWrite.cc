
#include "myhpp.h" 
typedef std::vector<Foo> FooList;
typedef shared_ptr<FooList> FooListPtr;
MutexLock mutex;
FooListPtr g_foos;

void traverse()
{
    FooListPtr foos;
    {
        MutexLockGuard lock(mutex);
        foos = g_foos;
        assert(!g_foos.unique());
    }

    for (vector<Foo>::const_iterator it = foos->begin(); it != foos->end(); it++)
    {
        it->doit();
    }
}

void post(const Foo &f)
{
    printf("post\n");
    MutexLockGuard lock(mutex);
    if (!g_foos.unique())
    {
        g_foos.reset(new FooList(*g_foos));
        printf("copy the whole list\n");
    }

    assert(g_foos.unique());
    g_foos->push_back(f);
}