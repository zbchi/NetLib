#include "myhpp.h"
class CustomerData
{
private:
    typedef std::pair<string, int> Entry;
    typedef std::vector<Entry> EntryList;
    typedef std::map<string, EntryList> Map;
    typedef std::shared_ptr<Map> MapPtr;
    void update(const string &customer, const EntryList &entries);

    static int findEntry(const EntryList &entries, const string &stock);
    MapPtr getData() const
    {
        MutexLockGuard lock(mutex_);
        return data_;
    }

    mutable MutexLock mutex_;
    MapPtr data_;

public:
    CustomerData() : data_(new Map) {}

    int query(const string &customer, const string &stock) const;
};

int CustomerData::query(const string &customer, const string &stock) const
{
    MapPtr data = getData();
    // 读时不加锁，只有get ptr时加锁,计数器加1,保留副本

    Map::const_iterator entries = data->find(customer);
    if (entries != data->end())
        return findEntry(entries->second, stock);
    else
        return -1;
}

void CustomerData::update(const string &customer, const EntryList &entries)
{
    MutexLockGuard lock(mutex_);
    if (!data_.unique())
    {
        MapPtr newData(new Map(*data_));
        data_.swap(newData);
    }
    assert(data_.unique());
    (*data_)[customer] = entries;
}