#pragma once
#include <memory>


namespace chronos
{
    template <typename ScheduleT, typename ExecuteT>
    class Dispatcher
    {
    public:
        using schedule_t = ScheduleT;
        using schedule_ptr = std::shared_ptr<schedule_t>;
        using time_duration_t = typename ScheduleT::duration_t;
    
        explicit Dispatcher(schedule_ptr schedule) : schedule(schedule) { }
        
        time_duration_t timeToNextTask() const
        {
            return schedule->timeToNextTask();
        }
    
        void handleNextTask()
        {
            auto task { schedule->withdrawNextTask() };
            const bool execution_succeed { execute(task.command) };
            if (!execution_succeed && has_attempts_left(task))
                schedule->retry(task);
            if (!is_retry(task))
                schedule->reschedule(task);
        }

    private:
        ExecuteT execute;
        schedule_ptr schedule;
    };
}
