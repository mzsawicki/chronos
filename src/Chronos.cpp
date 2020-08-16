#include <csignal>
#include "fmt/color.h"
#include "chronos/Coordinator.hpp"
#include "chronos/Dispatcher.hpp"
#include "chronos/FileReader.hpp"
#include "chronos/Logging.hpp"
#include "chronos/Parser.hpp"
#include "chronos/Schedule.hpp"
#include "chronos/System.hpp"
#include "chronos/Task.hpp"


namespace chronos
{
    using path_t = std::filesystem::path;
    using second_clock_t = boost::posix_time::second_clock;
    using schedule_t = ScheduleLoggingProxy<Schedule<Task, second_clock_t> >;
    using schedule_ptr_t = std::shared_ptr<schedule_t>;
    using system_call_t = SystemCallLoggingProxy<SystemCall>;
    using dispatcher_t = Dispatcher<schedule_t, system_call_t>;
    using coordinator_t = Coordinator<dispatcher_t>;
    using task_buidler_t = TaskBuilder<second_clock_t>;
    using parser_t = Parser<task_buidler_t>;
    using file_reader_t = FileReader<parser_t, schedule_t>;
}

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
    schedule_ptr_t parse_schedule(const path_t &file_path)
    {
        file_reader_t fileReader;
        return fileReader.read(file_path);
    }

    void print_error_message(const std::string &message)
    {
        constexpr auto FOREGROUND_COLOR { fmt::color::crimson };
        constexpr auto TEXT_EMPHASIS { fmt::emphasis::bold };
        const auto error_message_formatting {
                fg(FOREGROUND_COLOR) | TEXT_EMPHASIS };
        fmt::print(error_message_formatting, message);
    }

    void show_arg_count_error(const error::WrongNumberOfArguments &error)
    {
        print_error_message(error.what());
    }

    void show_syntax_error(const parser::error::SyntaxError &error)
    {
        print_error_message(fmt::format(
                "Syntax error: {}", error.what()));
    }

    coordinator_t setup_coordinator(schedule_ptr_t schedule)
    {
        dispatcher_t dispatcher(schedule);
        return coordinator_t(dispatcher);
    }

    void validate_arguments_count(int argc)
    {
        constexpr auto CORRECT_ARGC { 2 };
        if (argc != CORRECT_ARGC)
            throw error::WrongNumberOfArguments(--argc);
    }

    std::filesystem::path read_file_path_from_arg(int argc, char **argv)
    {
        validate_arguments_count(argc);
        constexpr auto SOURCE_FILE_ARG { 1 };
        const auto source_path { argv[SOURCE_FILE_ARG] };
        return std::filesystem::path(source_path);
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

namespace chronos::status
{
    constexpr int OK { 0 };
    constexpr int ERROR { 1 };
}

namespace chronos
{
    int run(int argc, char **argv)
    {
        // Setup logger
        setup_file_logger();

        // Get tasks file path
        path_t tasks_file_path;
        try {
            tasks_file_path = detail::read_file_path_from_arg(argc, argv);
        }
        catch (const error::WrongNumberOfArguments &error) {
            detail::show_arg_count_error(error);
            return status::ERROR;
        }

        // Load schedule from file
        schedule_ptr_t schedule;
        try {
            schedule = detail::parse_schedule(tasks_file_path);
        }
        catch (const parser::error::SyntaxError &error) {
            detail::show_syntax_error(error);
            return status::ERROR;
        }

        // Setup signal handling
        void (*interrupt_handler) (int);
        interrupt_handler = signal(SIGINT, signals::interrupt);

        // Create coordinator
        auto coordinator { detail::setup_coordinator(schedule) };

        return status::OK;
    }
}

int main(int argc, char **argv)
{
    return chronos::run(argc, argv);
}