#pragma once
#include <variant>
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"


namespace chronos
{
    using command_t = std::string;
    using retry_count_t = int;
    using time_t = boost::posix_time::ptime;
    using time_duration_t = boost::posix_time::time_duration;
    using days_duration_t = boost::gregorian::date_duration;
    using weeks_duration_t = boost::gregorian::weeks;
    using months_duration_t = boost::gregorian::months;
    using duration_t = std::variant<time_duration_t, days_duration_t,
        weeks_duration_t, months_duration_t>;
}

namespace chronos::time
{
    time_t transit(const time_t &time_point, const time_duration_t &duration)
    {
        return time_t(time_point + duration);
    }

    time_t transit(const time_t &time_point, const days_duration_t &duration)
    {
        const auto date { time_point.date() };
        const auto offset { time_point.time_of_day() };
        return time_t(date + duration, offset);
    }

    time_t transit(const time_t &time_point, const weeks_duration_t &duration)
    {
        const auto date { time_point.date() };
        const auto offset { time_point.time_of_day() };
        return time_t(date + duration, offset);
    }

    time_t transit(const time_t &time_point, const months_duration_t &duration)
    {
        const auto date { time_point.date() };
        const auto offset { time_point.time_of_day() };
        return time_t(date + duration, offset);
    }
}

namespace chronos
{
    struct Task
    {
        command_t command;
        time_t time;
        duration_t interval;
        retry_count_t attempts_count { 0 };
        retry_count_t max_retries_count { 0 };
        time_duration_t retry_after;
    };

    bool is_retry(const Task &task)
    {
        return task.attempts_count > 0;
    }

    bool has_attempts_left(const Task &task)
    {
        return task.attempts_count < task.max_retries_count;
    }

    void transit(Task &task)
    {
        const auto time_transition {[&task] (const auto duration) {
            return time::transit(task.time, duration); } };

        task.time = std::visit(time_transition, task.interval);
        task.attempts_count = 0;
    }

    Task create_retry(const Task &task)
    {
        Task retry_task(task);
        retry_task.time += task.retry_after;
        retry_task.attempts_count += 1;
        return retry_task;
    }
}

namespace chronos::task::compare
{
    struct Later
    {
        bool operator() (const Task &lhs, const Task &rhs)
        {
            return lhs.time > rhs.time;
        }
    };
}