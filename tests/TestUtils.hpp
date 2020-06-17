#pragma once
#include "boost/date_time/posix_time/posix_time_types.hpp"


namespace test
{
    struct Clock
    {
        using timepoint_t = boost::posix_time::ptime;
        using duration_t = boost::posix_time::time_duration;

        inline static timepoint_t time;

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

    template <typename ScheduleT>
    std::string pop_command(ScheduleT &schedule)
    {
        auto task { schedule.withdrawNextTask() };
        return task.command;
    }
}
