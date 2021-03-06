#include <filesystem>
#include <iostream>
#include <string>
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "chronos/Dispatcher.hpp"
#include "chronos/Filesystem.hpp"
#include "chronos/Logging.hpp"
#include "chronos/Parser.hpp"
#include "chronos/Schedule.hpp"
#include "chronos/Task.hpp"
#include "TestUtils.hpp"


namespace simulation
{
    using task_t = chronos::Task;
    using test_clock_t = test::Clock;
    using task_builder_t = chronos::TaskBuilder<test_clock_t>;
    using parser_t = chronos::Parser<task_builder_t>;
    using schedule_t = chronos::ScheduleLoggingProxy<
            chronos::Schedule<task_t, test_clock_t> >;
    using execution_t = chronos::SystemCallLoggingProxy<test::FakeSystemCall>;
    using dispatcher_t = chronos::Dispatcher<schedule_t, execution_t>;
}

std_filesystem::path get_file_path_from_user()
{
    std::cout << "File path:\n> ";
    std::string input_path;
    std::cin >> input_path;
    std_filesystem::path path(input_path);
    return path;
}

void loop(simulation::dispatcher_t &dispatcher)
{
    const auto time_to_next_task { dispatcher.timeToNextTask() };
    simulation::test_clock_t::wait(time_to_next_task);
    dispatcher.handleNextTask();
    std::cin.get();
}

int main()
{
    chronos::setup_console_logger<simulation::test_clock_t>();
    simulation::test_clock_t::time =
            boost::posix_time::second_clock::local_time();

    const auto file { get_file_path_from_user() };

    const auto schedule {
        chronos::read_schedule_file<simulation::parser_t,
            simulation::schedule_t>(file) };
    simulation::dispatcher_t dispatcher(schedule);

    std::cin.get();
    while(!schedule->isEmpty())
        loop(dispatcher);

    return 0;
}