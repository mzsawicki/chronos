#include <filesystem>
#include <iostream>
#include <string>
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "chronos/Dispatcher.hpp"
#include "chronos/FileReader.hpp"
#include "chronos/Logging.hpp"
#include "chronos/Parser.hpp"
#include "chronos/Queue.hpp"
#include "chronos/Schedule.hpp"
#include "chronos/Task.hpp"
#include "TestUtils.hpp"


using task_t = chronos::Task;
using test_clock_t = test::Clock;
using task_builder_t = chronos::TaskBuilder<test_clock_t>;
using parser_t = chronos::Parser<task_builder_t>;
using queue_t = chronos::ThreadsafePriorityQueue<task_t,
        chronos::task::compare::Later>;
using schedule_t = chronos::ScheduleLoggingProxy<
        chronos::Schedule<queue_t, test_clock_t> >;
using execution_t = chronos::SystemCallLoggingProxy<
        test::FakeSystemCall>;
using dispatcher_t = chronos::Dispatcher<schedule_t, execution_t>;
using file_reader_t = chronos::FileReader<parser_t, schedule_t>;

std::filesystem::path get_file_path_from_user()
{
    std::string input_path;
    std::cin >> input_path;
    std::filesystem::path path(input_path);
    return path;
}

void loop(dispatcher_t &dispatcher)
{
    const auto time_to_next_task { dispatcher.timeToNextTask() };
    test_clock_t::wait(time_to_next_task);
    dispatcher.handleNextTask();
    std::cin.get();
}

int main()
{
    chronos::setup_console_logger<test_clock_t>();
    test_clock_t::time = boost::posix_time::second_clock::local_time();

    std::cout << "File path:\n> ";
    const auto file { get_file_path_from_user() };

    file_reader_t reader;
    const auto schedule { reader.read(file) };
    dispatcher_t dispatcher(schedule);

    while(!schedule->isEmpty())
        loop(dispatcher);

    return 0;
}

