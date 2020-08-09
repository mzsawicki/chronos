#pragma once
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <queue>
#include <vector>


namespace chronos
{
    template <typename TaskT, typename ClockT>
    class Schedule
    {
    public:
        using duration_t = boost::posix_time::time_duration;
        using task_t = TaskT;

        [[nodiscard]] bool isEmpty() const
        {
            return queue.empty();
        }

        void add(const TaskT &task)
        {
            queue.push(task);
        }

        void reschedule(TaskT &task)
        {
            transit(task);
            queue.push(task);
        }

        void retry(const TaskT &task)
        {
            auto retry_task { create_retry(task) };
            queue.push(retry_task);
        }

        [[nodiscard]] duration_t timeToNextTask() const
        {
            const auto task { queue.top() };
            return task.time - ClockT::local_time();
        }

        TaskT withdrawNextTask()
        {
            auto task { queue.top() };
            queue.pop();
            return task;
        }

    private:
        std::priority_queue<TaskT, std::vector<TaskT> > queue;
    };
}
