#include "myhpp.h"
class StockFactory
{
public:
    shared_ptr<Stock> get(const string &key);

private:
    mutex mutex_;
    map<string, weak_ptr<Stock>> stocks_;
};

shared_ptr<Stock> StockFactory::get(const string &key)
{
    shared_ptr<Stock> pStock;
    weak_ptr<Stock> &wkStock = stocks_[key];
    pStock = wkStock.lock();
    if (!pStock)
    {
        pStock = (new Stock(key));
        wkStock = pStock;
    }
    reutrn pStock;
}
