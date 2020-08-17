#pragma once
#include <variant>
#include <cstdlib>
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"


namespace chronos
{
    using command_t = std::string;
    using month_t = boost::gregorian::greg_month;
    using retry_count_t = int;
    using time_t = boost::posix_time::ptime;
    using date_t = boost::gregorian::date;
    using time_duration_t = boost::posix_time::time_duration;
    using seconds_duration_t = boost::posix_time::seconds;
    using minutes_duration_t = boost::posix_time::minutes;
    using hours_duration_t = boost::posix_time::hours;
    using days_duration_t = boost::gregorian::date_duration;
    using weeks_duration_t = boost::gregorian::weeks;
    using months_duration_t = boost::gregorian::months;
    using duration_t = std::variant<time_duration_t, days_duration_t,
        weeks_duration_t, months_duration_t>;
}

namespace chronos::time::constants
{
    constexpr auto MINUTES_IN_HOUR { 60 };
    constexpr auto HOURS_IN_DAY { 24 };
    constexpr auto DAYS_IN_WEEK { 7 };

    constexpr auto NO_MINUTES { 0 };
    constexpr auto NO_SECONDS { 0 };
}

namespace chronos::time
{
    using namespace constants;

    struct MonthTime
    {
        int day { 1 };
        int hour { 0 };
        int minute { 0 };
    };

    struct WeekTime
    {
        int day { 1 };
        int hour { 0 };
        int minute { 0 };
    };

    struct DayTime
    {
        int hour { 0 };
        int minute { 0 };
    };

    using hour_time_t = int;

    int minutes_count(const MonthTime &time)
    {
        return time.day * HOURS_IN_DAY * MINUTES_IN_HOUR
            + time.hour * MINUTES_IN_HOUR
            + time.minute;
    }

    int minutes_count(const WeekTime &time)
    {
        return time.day * HOURS_IN_DAY * MINUTES_IN_HOUR
            + time.hour * MINUTES_IN_HOUR
            + time.minute;
    }

    int minutes_count(const DayTime &time)
    {
        return time.hour * MINUTES_IN_HOUR
            + time.minute;
    }

    bool operator < (const MonthTime &lhs, const MonthTime &rhs)
    {
        return minutes_count(lhs) < minutes_count(rhs);
    }

    bool operator < (const WeekTime &lhs, const WeekTime &rhs)
    {
        return minutes_count(lhs) < minutes_count(rhs);
    }

    bool operator < (const DayTime &lhs, const DayTime &rhs)
    {
        return minutes_count(lhs) < minutes_count(rhs);
    }

    month_t increment(const month_t &month)
    {
        return month_t(month.as_number() % 12 + 1);
    }

    MonthTime extract_month_time(const time_t &time_point)
    {
        const auto date { time_point.date() };
        const auto time_of_day { time_point.time_of_day() };
        MonthTime month_time;
        month_time.day = date.day();
        month_time.hour = time_of_day.hours();
        month_time.minute = time_of_day.minutes();
        return month_time;
    }

    WeekTime extract_week_time(const time_t &time_point)
    {
        const auto date { time_point.date() };
        const auto time_of_day { time_point.time_of_day() };
        WeekTime week_time;
        week_time.day = date.day_of_week();
        week_time.hour = time_of_day.hours();
        week_time.minute = time_of_day.minutes();
        return week_time;
    }

    DayTime extract_day_time(const time_t &time_point)
    {
        const auto time_of_day { time_point.time_of_day() };
        DayTime day_time;
        day_time.hour = time_of_day.hours();
        day_time.minute = time_of_day.minutes();
        return day_time;
    }

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

    template <typename ClockT>
    time_t closest_future_time_point(const MonthTime &month_time)
    {
        const auto current_time { ClockT::local_time() };
        const auto current_date { current_time.date() };
        const auto current_month { current_date.month() };
        const auto current_year { current_date.year() };
        const auto current_month_time { extract_month_time(current_time) };
        const auto result_month {
            current_month_time < month_time
            ? current_month : increment(current_month) };
        const auto result_daytime {
            time_duration_t(month_time.hour, month_time.minute, NO_SECONDS) };
        const auto result_date {
            date_t(current_year, result_month, month_time.day) };
        return time_t(result_date, result_daytime);
    }

    template <typename ClockT>
    time_t closest_future_time_point(const WeekTime &week_time)
    {
        using constants::DAYS_IN_WEEK;
        const auto current_time { ClockT::local_time() };
        const auto current_date { current_time.date() };
        const auto current_week_time { extract_week_time(current_time) };
        const auto days_remaining {
            (DAYS_IN_WEEK - current_week_time.day + week_time.day)
            % DAYS_IN_WEEK };
        const auto result_daytime {
            time_duration_t(week_time.hour, week_time.minute, NO_SECONDS) };
        const auto result_date {
            current_date + days_duration_t(days_remaining)};
        return time_t(result_date, result_daytime);
    }

    template <typename ClockT>
    time_t closest_future_time_point(const DayTime &day_time)
    {
        const auto current_time { ClockT::local_time() };
        const auto current_date { current_time.date() };
        const auto current_hour_time {extract_day_time(current_time) };
        const auto result_daytime {
            time_duration_t(day_time.hour, day_time.minute, NO_SECONDS) };
        const auto result_date {
            current_hour_time < day_time
            ? current_date : current_date + days_duration_t(1)};
        return time_t(result_date, result_daytime);
    }

    template <typename ClockT>
    time_t closest_future_time_point(hour_time_t hour_time)
    {
        const auto current_time { ClockT::local_time() };
        const auto current_daytime { current_time.time_of_day() };
        const auto current_date { current_time.date() };
        const auto current_hour { current_daytime.hours() };
        const auto result_time { time_t(current_date,
                time_duration_t(current_hour, hour_time, NO_SECONDS)) };
        return current_time < result_time
            ? result_time : result_time + hours_duration_t(1);
    }

    template <typename ClockT>
    time_t closest_future_time_point()
    {
        const auto current_time { ClockT::local_time() };
        const auto current_daytime { current_time.time_of_day() };
        const auto current_date { current_time.date() };
        const auto current_hour { current_daytime.hours() };
        const auto current_minute { current_daytime.minutes() };
        return time_t(current_date,
                time_duration_t(current_hour, current_minute + 1, NO_SECONDS));
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

    bool operator < (const Task &lhs, const Task &rhs)
    {
        return lhs.time > rhs.time;
    }

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

namespace chronos
{
    template <typename ClockT>
    class TaskBuilder
    {
    public:
        using task_t = Task;

        TaskBuilder& createTask()
        {
            task = Task();
            return *this;
        }

        TaskBuilder& withCommand(const std::string &command)
        {
            task.command = command;
            return *this;
        }

        TaskBuilder& everyMonthsCount(int months)
        {
            task.interval = months_duration_t(months);
            return *this;
        }

        TaskBuilder& everyWeeksCount(int weeks)
        {
            task.interval = weeks_duration_t(weeks);
            return *this;
        }

        TaskBuilder& everyDaysCount(int days)
        {
            task.interval = days_duration_t(days);
            return *this;
        }

        TaskBuilder& everyHoursCount(int hours)
        {
            task.interval = time_duration_t(
                    hours, time::constants::NO_MINUTES,
                    time::constants::NO_SECONDS);
            return *this;
        }

        TaskBuilder& everyMinutesCount(int minutes)
        {
            task.interval = minutes_duration_t(minutes);
            return *this;
        }

        TaskBuilder& atMonthDay(const time::MonthTime &time)
        {
            task.time = time::closest_future_time_point<ClockT>(time);
            return *this;
        }

        TaskBuilder& atWeekDay(const time::WeekTime &time)
        {
            task.time = time::closest_future_time_point<ClockT>(time);
            return *this;
        }

        TaskBuilder& atHour(const time::DayTime &time)
        {
            task.time = time::closest_future_time_point<ClockT>(time);
            return *this;
        }

        TaskBuilder& atMinute(time::hour_time_t time)
        {
            task.time = time::closest_future_time_point<ClockT>(time);
            return *this;
        }

        TaskBuilder& atMinute()
        {
            task.time = time::closest_future_time_point<ClockT>();
            return *this;
        }

        TaskBuilder& retryTimes(int count)
        {
            task.max_retries_count = count;
            return *this;
        }

        TaskBuilder& retryAfter(int seconds)
        {
            task.retry_after = seconds_duration_t(seconds);
            return *this;
        }

        Task build() const
        {
            return task;
        }

    private:
        Task task;
    };
}