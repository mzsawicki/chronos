#pragma once
#include <memory>

namespace chronos::dispatcher::detail
{
    template <typename ScheduleT>
    void move_retries(std::shared_ptr<ScheduleT> &old,
                      std::shared_ptr<ScheduleT> &new_)
    {
        for (auto task = old->withdrawNextTask(); !old->isEmpty();
            task = old->withdrawNextTask())
            if (is_retry(task))
                new_->add(task);
    }
}

namespace chronos
{
    template <typename ScheduleT, typename ExecuteT>
    class Dispatcher
    {
    public:
        using schedule_t = ScheduleT;
        using schedule_ptr_t = std::shared_ptr<schedule_t>;
        using time_duration_t = typename ScheduleT::duration_t;

        explicit Dispatcher(schedule_ptr_t schedule) : schedule(schedule) { }

        time_duration_t timeToNextTask() const
        {
            return schedule->timeToNextTask();
        }

        void handleNextTask()
        {
            auto task { schedule->withdrawNextTask() };
            const auto execution_response { execute(task.command) };
            const bool execution_succeed { execution_response.success };
            if (!execution_succeed && has_attempts_left(task))
                schedule->retry(task);
            if (!is_retry(task))
                schedule->reschedule(task);
        }

        void reload(schedule_ptr_t new_schedule)
        {
            dispatcher::detail::move_retries(schedule, new_schedule);
            schedule = new_schedule;
        }

    private:
        ExecuteT execute;
        schedule_ptr_t schedule;
    };
}
