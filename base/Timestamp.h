#pragma once
#include <string>
#include <cstdint>
namespace mylib
{

    class Timestamp
    {
    private:
        int64_t microSecondsSinceEpoch_;

    public:
        Timestamp() : microSecondsSinceEpoch_(0)
        {
        }
        explicit Timestamp(int64_t microSecondsSinceEpochArg)
            : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
        {
        }
        void swap(Timestamp &that)
        {
            std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
        }

        std::string toString() const;
        std::string toFormattedString(bool showMicroseconds = true) const;
        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
        time_t secondsSinceEpoch() const
        {
            return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
        }

        static Timestamp now();
        static Timestamp invalid() { return Timestamp(); }

        static Timestamp fromUnixTime(time_t t) { return fromUnixTime(t, 0); }
        static Timestamp fromUnixTime(time_t t, int microseconds)
        {
            return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
        }
        static const int kMicroSecondsPerSecond = 1000 * 1000;
    };

    inline mylib::Timestamp addTime(mylib::Timestamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * mylib::Timestamp::kMicroSecondsPerSecond);
        return mylib::Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }

    inline bool operator<(mylib::Timestamp lhs,mylib::Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(mylib::Timestamp lhs, mylib::Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }
};