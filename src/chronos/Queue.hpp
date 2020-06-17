#pragma once
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <vector>


namespace chronos
{
    template <typename ValueT, typename CompareT>
    class ThreadsafePriorityQueue
    {
    public:
        using value_t = ValueT;
        using container_t = std::vector<value_t>;
        using queue_t = std::priority_queue<value_t, container_t, CompareT>;
        
        bool empty() const
        {
            std::shared_lock lock(mutex);
            return queue.empty();
        }
        
        typename queue_t::const_reference top() const
        {  
            std::shared_lock lock(mutex);
            return queue.top();
        }
        
        void push(const value_t &val)
        {
            std::unique_lock lock(mutex);
            queue.push(val);
        }
        
        void pop()
        {
            std::unique_lock lock(mutex);
            queue.pop();
        }
        
    private:
        mutable std::shared_mutex mutex;
        queue_t queue;
    };
}
