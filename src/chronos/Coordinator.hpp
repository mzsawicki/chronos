#pragma once


namespace chronos
{
    template <typename DispatcherT, typename TimerT>
    class Coordinator
    {
    public:
        explicit Coordinator(DispatcherT dispatcher)
            : dispatcher(std::move(dispatcher)) { }

        void loop()
        {
            const auto time_to_next_task { dispatcher.timeToNextTask() };
            timer.wait(time_to_next_task);
            if (!terminated)
                dispatcher.handleNextTask();
        }

        void terminate()
        {
            terminated = true;
            timer.interrupt();
        }

    private:
        bool terminated { false };
        DispatcherT dispatcher;
        TimerT timer;
    };
}