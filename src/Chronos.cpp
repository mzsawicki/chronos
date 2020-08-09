#include "chronos/Coordinator.hpp"
#include "chronos/Dispatcher.hpp"
#include "chronos/FileReader.hpp"
#include "chronos/IO.hpp"
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

    constexpr int STATUS_OK { 0 };
    constexpr int STATUS_ERROR { 1 };

    schedule_ptr_t parse_schedule(const path_t &file_path)
    {
        file_reader_t fileReader;
        return fileReader.read(file_path);
    }

    void show_arg_count_error(const io::error::WrongNumberOfArguments &error)
    {
        chronos::show_error_message(error.what());
    }

    void show_syntax_error(const parser::error::SyntaxError &error)
    {
        chronos::show_error_message(fmt::format(
                "Syntax error: {}", error.what()));
    }

    coordinator_t setup_coordinator(schedule_ptr_t schedule)
    {
        dispatcher_t dispatcher(schedule);
        return coordinator_t(dispatcher);
    }
}

int main(int argc, char **argv)
{
    chronos::setup_file_logger();
    chronos::schedule_ptr_t schedule;

    chronos::path_t chronos_file_path;
    try {
        chronos_file_path = chronos::read_file_path(argc, argv);
    }
    catch (const chronos::io::error::WrongNumberOfArguments &error) {
        chronos::show_arg_count_error(error);
        return chronos::STATUS_ERROR;
    }

    try {
        schedule = chronos::parse_schedule(chronos_file_path);
    }
    catch (const chronos::parser::error::SyntaxError &error) {
        chronos::show_syntax_error(error);
        return chronos::STATUS_ERROR;
    }

    auto coordinator { chronos::setup_coordinator(schedule) };
    while (true)
        coordinator.loop();

    return chronos::STATUS_OK;
};