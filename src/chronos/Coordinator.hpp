#pragma once
#include <atomic>
#include <memory>
#include <thread>


namespace chronos::coordinator
{
    template <typename DispatcherT, typename TimerT>
    class Coordinator
    {
    public:
        using dispatcher_ptr_t = std::shared_ptr<DispatcherT>;

        explicit Coordinator(dispatcher_ptr_t dispatcher)
            : dispatcher(dispatcher) { }

        void loopForever()
        {
            while(!terminated)
                loop();
        }

        void terminate()
        {
            terminated = true;
            timer.interrupt();
        }

    private:
        void loop()
        {
            timer.wait(dispatcher->timeToNextTask());
            if (!terminated)
                dispatcher->handleNextTask();
        }

        std::atomic<bool> terminated { false };
        dispatcher_ptr_t dispatcher;
        TimerT timer;
    };
}

namespace chronos
{
    template <typename DispatcherT, typename TimerT>
    class CoordinatorThread
    {
    private:
        using dispatcher_ptr_t = std::shared_ptr<DispatcherT>;
        using coordinator_t = coordinator::Coordinator<DispatcherT, TimerT>;
        using coordinator_ptr_t = std::unique_ptr<coordinator_t>;
        using thread_ptr_t = std::unique_ptr<std::thread>;

    public:
        explicit CoordinatorThread(dispatcher_ptr_t dispatcher)
            : coordinator(create_coordinator(dispatcher)),
            thread(create_thread(coordinator)) { }

        void terminate()
        {
            coordinator->terminate();
            thread->join();
        }

    private:
        static coordinator_ptr_t
        create_coordinator(dispatcher_ptr_t dispatcher)
        {
            return std::make_unique<coordinator_t>(dispatcher);
        }

        static thread_ptr_t create_thread(coordinator_ptr_t &coordinator)
        {
            const auto run_coordinator {
                [&coordinator] () { coordinator->loopForever(); } };
            return std::make_unique<std::thread>(run_coordinator);
        }

        coordinator_ptr_t coordinator;
        thread_ptr_t thread;
    };
}