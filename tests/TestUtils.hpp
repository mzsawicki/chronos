#pragma once
#include <random>
#include "boost/date_time/posix_time/posix_time_types.hpp"


namespace test
{
    struct Clock
    {
        using timepoint_t = boost::posix_time::ptime;
        using duration_t = boost::posix_time::time_duration;

        inline static timepoint_t time {
            boost::posix_time::ptime(
                    boost::gregorian::date(1960, 1, 1))
        };

        static timepoint_t local_time()
        {
            return time;
        }

        static void wait(const duration_t &wait_duration)
        {
            time += wait_duration;
        }
    };

    struct FailingExecution
    {
        bool operator() (const std::string&)
        {
            return false;
        }
    };

    class FakeSystemCall
    {
    public:
        bool operator() (const std::string&)
        {
            return distribution(generator);
        }

    private:
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution {
            std::uniform_int_distribution(0, 1) };
    };

    template <typename ScheduleT>
    std::string pop_command(ScheduleT &schedule)
    {
        auto task { schedule.withdrawNextTask() };
        return task.command;
    }
}
