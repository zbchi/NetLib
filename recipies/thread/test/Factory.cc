#include "myhpp.h"
class Stock;
class Stock
{
private:
    string name_;

public:
    Stock(const string &name) : name_(name)
    {
        printf(" Stock[%p] %s\n", this, this->name_.c_str());
    }
    ~Stock()
    {
        printf("~Stock[%p] %s\n", this, this->name_.c_str());
    }
    string key()
    {
        return name_;
    }
};
class StockFactory : public std::enable_shared_from_this<StockFactory>
{
public:
    shared_ptr<Stock> get(const string &key);

private:
    mutex mutex_;
    map<string, weak_ptr<Stock>> stocks_;
    void weakDeleteCallback(const weak_ptr<StockFactory> &wkFactory, Stock *stock);
    void removeStock(Stock *stock);
};

shared_ptr<Stock> StockFactory::get(const string &key)
{
    shared_ptr<Stock> pStock;
    weak_ptr<Stock> &wkStock = stocks_[key];
    pStock = wkStock.lock();
    if (!pStock)
    {
        // 弱回调
        pStock.reset(new Stock(key),

                     [wkFactory = weak_ptr<StockFactory>(shared_from_this())](Stock *stock)
                     {
                         shared_ptr<StockFactory> factory(wkFactory.lock());
                         if (factory)
                         {
                             factory->removeStock(stock);
                         }
                         delete stock;
                     }

        );
        // 在这其他线程可能析构this，强回调延长了this的生命周期,将this变成shared_ptr
        // 将shared_ptr转型为weak_ptr才不会延长生命周期
        wkStock = pStock;
    }
    return pStock;
}

void StockFactory::removeStock(Stock *stock)
{
    if (stock)
    {
        lock_guard<mutex> lock(this->mutex_);
        stocks_.erase(stock->key());
    }
}

void testLongLifeFactory()
{
    shared_ptr<StockFactory> factory(new StockFactory);
    {
        shared_ptr<Stock> stock = factory->get("NYSE:IBM");
        shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");

        assert(stock == stock2);
        // stock destructs
    }
    // factory destructs
}

void testShortLifeFactory()
{
    shared_ptr<Stock> stock;
    {
        shared_ptr<StockFactory> factory(new StockFactory);
        stock = factory->get("NYSE:IBM");
        shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
        assert(stock == stock2);
        // factory destructs
    }
    // stock destructs
}

int main()
{
    testLongLifeFactory();
    testShortLifeFactory();
    return 0;
}