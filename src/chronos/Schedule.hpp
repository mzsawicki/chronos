#pragma once
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include <map>
#include <memory>


namespace chronos
{
    template <typename QueueT, typename ClockT>
    class Schedule
    {
    public:
        using task_t = typename QueueT::value_t;
        using duration_t = boost::posix_time::time_duration;

        [[nodiscard]] bool isEmpty() const
        {
            return queue.empty();
        }
    
        void add(const task_t &task)
        {
            queue.push(task);
        }

        void reschedule(task_t &task)
        {
            transit(task);
            queue.push(task);
        }

        void retry(const task_t &task)
        {
            auto retry_task { create_retry(task) };
            queue.push(retry_task);
        }

        [[nodiscard]] duration_t timeToNextTask() const
        {
            const auto task { queue.top() };
            return task.time - ClockT::local_time();
        }
        
        task_t withdrawNextTask()
        {
            auto task { queue.top() };
            queue.pop();
            return task;
        }
        
    private:
        QueueT queue;
    };

    template <typename ScheduleT>
    std::shared_ptr<ScheduleT> createSchedule()
    {
        return std::make_shared<ScheduleT>();
    }
}
