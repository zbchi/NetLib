#include "myhpp.h"
class Stock
{
};
class StockFactory
{
public:
    shared_ptr<Stock> get(const string &key);

private:
    mutex mutex_;
    map<string, weak_ptr<Stock>> stocks_;
    void deleteStock(Stock *stock);
};

shared_ptr<Stock> StockFactory::get(const string &key)
{
    shared_ptr<Stock> pStock;
    weak_ptr<Stock> &wkStock = stocks_[key];
    pStock = wkStock.lock();
    if (!pStock)
    {
        pStock.reset(new Stock, bind(&StockFactory::deleteStock, this, _1));
        wkStock = pStock;
    }
    return pStock;
}

void StockFactory::deleteStock(Stock *stock)
{
    if (stock)
    {
        lock_guard<mutex> lock(this->mutex_);
        stocks_.erase(stock->key());
    }
    delete stock;
}
