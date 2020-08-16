#pragma once
#include <chrono>
#include <condition_variable>
#include <mutex>
#include "boost/date_time/posix_time/posix_time_types.hpp"


namespace chronos
{
    class Timer
    {
    public:
        using duration_t = boost::posix_time::time_duration;
        using seconds_t = std::chrono::seconds;

        void wait(const duration_t &duration)
        {
            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);
            const auto seconds_wait { duration.total_seconds() };
            interrupted.wait_for(lock, seconds_t(seconds_wait));
        }

        void interrupt()
        {
            interrupted.notify_one();
        }

    private:
        std::condition_variable interrupted;
    };
}
