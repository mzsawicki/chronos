#pragma once
#include <random>
#include "boost/date_time/posix_time/posix_time_types.hpp"


namespace test::system
{
    struct Response
    {
        bool success;
        std::string message;
    };
}

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
        using response_t = system::Response;

        response_t operator() (const std::string&)
        {
            return { .success = false };
        }
    };

    class FakeSystemCall
    {
    public:
        using response_t = system::Response;

        response_t operator() (const std::string&)
        {
            const bool success(distribution(generator));
            return { .success = success };
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
