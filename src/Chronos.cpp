#include <csignal>
#include <cstdlib>
#include "fmt/color.h"
#include "chronos/Coordinator.hpp"
#include "chronos/Dispatcher.hpp"
#include "chronos/Filesystem.hpp"
#include "chronos/Logging.hpp"
#include "chronos/Parser.hpp"
#include "chronos/Schedule.hpp"
#include "chronos/System.hpp"
#include "chronos/Task.hpp"
#include "chronos/Timer.hpp"


namespace chronos::error
{
    class WrongNumberOfArguments : public std::runtime_error
    {
    public:
        explicit WrongNumberOfArguments(int args_count)
                : std::runtime_error(fmt::format(
                "Invalid number of arguments: {}", args_count)) { }
    };
}

namespace chronos::detail
{
    void validate_arguments_count(int argc)
    {
        constexpr auto CORRECT_ARGC { 2 };
        if (argc != CORRECT_ARGC)
            throw error::WrongNumberOfArguments(--argc);
    }
}

namespace chronos
{
    using clock_t_ = boost::posix_time::second_clock;
    using schedule_t = ScheduleLoggingProxy<Schedule<Task, clock_t_> >;
    using system_call_t = SystemCallLoggingProxy<SystemCall>;
    using dispatcher_t = DispatcherLoggingProxy<
            Dispatcher<schedule_t, system_call_t> >;
    using task_buidler_t = TaskBuilder<clock_t_>;
    using parser_t = Parser<task_buidler_t>;
    using logging_parser_t = ParserLoggingProxy<parser_t>;
    using coordinator_t = CoordinatorThread<dispatcher_t, Timer>;
    using file_lock_t = FileLock<Timer>;

    void print_error_message(const std::string &message)
    {
        constexpr auto FOREGROUND_COLOR { fmt::color::crimson };
        constexpr auto TEXT_EMPHASIS { fmt::emphasis::bold };
        const auto error_message_formatting {
                fg(FOREGROUND_COLOR) | TEXT_EMPHASIS };
        fmt::print(error_message_formatting, message);
    }
}

namespace chronos::program
{
    std::shared_ptr<dispatcher_t>
    setup_dispatcher(const std_filesystem::path &file)
    {
        auto schedule { read_schedule_file<parser_t, schedule_t>(file) };
        return std::make_shared<dispatcher_t>(schedule);
    }

    std::unique_ptr<file_lock_t>
    setup_file_lock(const std::filesystem::path &file)
    {
        return std::make_unique<file_lock_t>(file);
    }

    class Program
    {
    private:
        struct Context
        {
            explicit Context(const std_filesystem::path &path)
                : dispatcher(setup_dispatcher(path)),
                lock(setup_file_lock(path)) { }

            std::shared_ptr<dispatcher_t> dispatcher;
            std::unique_ptr<file_lock_t> lock;
        };

    public:
        explicit Program(const std_filesystem::path &path)
            : source_file(path),
            context(path) { }

        void run()
        {
            while (!stopped)
                loop();
        }

        void stop()
        {
            stopped = true;
            context.lock->release();
        }

    private:
        void loop()
        {
            coordinate();
            if (!stopped)
                reload();
        }

        void coordinate() const
        {
            using seconds_t = boost::posix_time::seconds;
            constexpr int FILE_CHECK_INTERVAL { 60 };
            coordinator_t coordinator(context.dispatcher);
            context.lock->waitUntilChange(seconds_t(FILE_CHECK_INTERVAL));
            coordinator.terminate();
        }

        void reload()
        {
            try {
                auto new_schedule {
                    read_schedule_file<logging_parser_t , schedule_t>(
                            source_file) };
                context.dispatcher->reload(new_schedule);
            } catch (...) { }
        }

        std::atomic<bool> stopped { false };
        std_filesystem::path source_file;
        Context context;
    };

    std::unique_ptr<Program> setup_program(const std_filesystem::path &path)
    {
        return std::make_unique<Program>(path);
    }

    std::unique_ptr<std::thread> run_thread(std::unique_ptr<Program> &program)
    {
        const auto run_program { [&program] () { program->run(); } };
        return std::make_unique<std::thread>(run_program);
    }
}

namespace chronos
{
    class MainThread
    {
    public:
        explicit MainThread(const std_filesystem::path &path)
                : program(program::setup_program(path)),
                  thread(run_thread(program)) { }

        void terminate()
        {
            program->stop();
            thread->join();
        }

    private:
        std::unique_ptr<program::Program> program;
        std::unique_ptr<std::thread> thread;
    };

    std_filesystem::path read_file_path_from_arg(int argc, char **argv)
    {
        detail::validate_arguments_count(argc);
        constexpr auto SOURCE_FILE_ARG { 1 };
        const auto source_path { argv[SOURCE_FILE_ARG] };
        return std::filesystem::path(source_path);
    }

    std::unique_ptr<MainThread> run(const std_filesystem::path &path)
    {
        return std::make_unique<MainThread>(path);
    }

    void wait_for_interrupt()
    {
        using seconds_t = boost::posix_time::seconds;
        Timer().wait(seconds_t(1));
    }
}

namespace chronos::signals
{
    volatile sig_atomic_t interrupted { 0 };

    void interrupt(int)
    {
        interrupted = 1;
    }
}

int main(int argc, char **argv)
{
    void (*interrupt_handler) (int);
    interrupt_handler = signal(SIGINT, chronos::signals::interrupt);

    chronos::setup_file_logger();

    std::unique_ptr<chronos::MainThread> main_thread;
    try {
        auto path { chronos::read_file_path_from_arg(argc, argv) };
        main_thread = chronos::run(path);
    } catch (const std::exception &error) {
        chronos::print_error_message(error.what());
        return EXIT_FAILURE;
    }

    while (!chronos::signals::interrupted)
        chronos::wait_for_interrupt();
    main_thread->terminate();

    return EXIT_SUCCESS;
}