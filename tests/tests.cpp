#define CATCH_CONFIG_MAIN
#include "boost/date_time/posix_time/posix_time.hpp"
#include "catch2/catch.hpp"
#include "chronos/Dispatcher.hpp"
#include "chronos/Parser.hpp"
#include "chronos/Schedule.hpp"
#include "chronos/System.hpp"
#include "chronos/Task.hpp"
#include "TestUtils.hpp"


namespace chronos
{
    using second_clock_t = boost::posix_time::second_clock;
    using schedule_t = chronos::Schedule<chronos::Task, second_clock_t>;
}

namespace test
{
    using artificial_clock_t = test::Clock;
}

SCENARIO ("Scheduled tasks are sorted chronologically", "[unit]")
{
    GIVEN ("An empty schedule")
    {
        chronos::schedule_t schedule;
        
        WHEN ("Three tasks are added in shuffled order")
        {
            using namespace boost::gregorian;
            using namespace boost::posix_time;

            chronos::Task task1;
            task1.command = "third";
            task1.time = ptime(date(2020, Jan, 3),
                    hours(1));

            chronos::Task task2;
            task2.command = "first";
            task2.time = ptime(date(2020, Jan, 1),
                    hours(1));

            chronos::Task task3;
            task3.command = "second";
            task3.time = ptime(date(2020, Jan, 2),
                    hours(1));

            schedule.add(task1);
            schedule.add(task2);
            schedule.add(task3);

            THEN ("Jobs are withdrawn in correct chronological order")
            {
                const bool order_correct {
                    test::pop_command(schedule) == "first"
                    && test::pop_command(schedule) == "second"
                    && test::pop_command(schedule) == "third"
                };

                REQUIRE(order_correct);
            }
        }
    }
}

SCENARIO ("Time transition is carried correctly", "[unit]")
{
    GIVEN ("A task with certain execution time and interval")
    {
        using namespace boost::gregorian;
        using namespace boost::posix_time;

        chronos::Task task;
        task.time = ptime(date(2021, Feb, 22), hours(1));
        task.interval = hours(2) + minutes(30);

        WHEN ("Execution time transition is done")
        {
            chronos::transit(task);

            THEN ("New time is correct")
            {
                const auto correct_new_time {
                    ptime(date(2021, Feb, 22),
                            hours(3)) + minutes(30) };

                REQUIRE(task.time == correct_new_time);
            }
        }
    }
}

SCENARIO ("Week transition is carried correctly", "[unit]")
{
    GIVEN ("A task with certain execution time and date interval")
    {
        using namespace boost::gregorian;
        using namespace boost::posix_time;

        chronos::Task task;
        task.time = ptime(date(2021, Jan, 14), hours(1));
        task.interval = weeks(1);

        WHEN ("Execution time transition is done")
        {
            chronos::transit(task);

            THEN ("New time is correct")
            {
                const auto correct_new_time {
                    ptime(date(2021, Jan, 21), hours(1)) };

                REQUIRE(task.time == correct_new_time);
            }
        }
    }
}

SCENARIO ("Month transition is carried correctly", "[unit]")
{
    GIVEN ("A task with certain execution time and date interval")
    {
        using namespace boost::gregorian;
        using namespace boost::posix_time;

        chronos::Task task;
        task.time = ptime(date(2021, Dec, 30), hours(10));
        task.interval = months(3);

        WHEN ("Execution time transition is done")
        {
            chronos::transit(task);

            THEN ("New time is correct")
            {
                const auto correct_new_time {
                    ptime(date(2022, Mar, 30), hours(10)) };

                REQUIRE(task.time == correct_new_time);
            }
        }
    }
}

SCENARIO ("Failed job is retried exact number of times", "[unit]")
{
    using failing_dispatcher_t = chronos::Dispatcher<chronos::schedule_t,
        test::FailingExecution>;
    auto schedule { std::make_shared<chronos::schedule_t>() };
    failing_dispatcher_t dispatcher(schedule);

    GIVEN ("Task with 3 max retries")
    {
        using namespace boost::gregorian;
        using namespace boost::posix_time;

        chronos::Task task;
        task.time = ptime(date(2020, Jul, 1), hours(12));
        task.interval = days(1);
        task.retry_after = seconds(10);
        task.max_retries_count = 3;
        schedule->add(task);

        WHEN ("Execution fails 4 times")
        {
            for (int i = 0; i < 4; ++i)
                dispatcher.handleNextTask();

            THEN ("There remains only one task in the schedule")
            {
                schedule->withdrawNextTask();
                REQUIRE(schedule->isEmpty());
            }
        }
    }
}

SCENARIO ("Entry with retry parameters is parsed correctly", "[unit]")
{
    using parser_t = chronos::parser::parser;
    using boost::spirit::ascii::space;
    using chronos::parser::strct::TaskEntry;
    parser_t parser;

    TaskEntry output;

    GIVEN ("Entry with retry time and count")
    {
        const std::string entry {
            "Run \"test:test\" every 3 hours retry after 5 seconds "
            "3 times;" };

        WHEN ("Entry is parsed")
        {
            std::string::const_iterator iter { entry.begin() };
            std::string::const_iterator end { entry.end() };
            const bool result {
                phrase_parse(iter, end, parser, space, output) };

            THEN ("Parsing is successful")
            {
                const bool success { result && iter == end };
                REQUIRE(success);
            }
        }
    }
}

SCENARIO ("Entry with specified time is parsed correctly", "[unit]")
{
    using parser_t = chronos::parser::parser;
    using boost::spirit::ascii::space;
    using chronos::parser::strct::TaskEntry;
    parser_t parser;

    TaskEntry output;

    GIVEN ("Entry with 'at' part")
    {
        const std::string entry {
                "Run \"./program -i --param\" every month at 2 12:30"
                " retry after 5 seconds 3 times;" };

        WHEN ("Entry is parsed")
        {
            std::string::const_iterator iter { entry.begin() };
            std::string::const_iterator end { entry.end() };
            const bool result {
                    phrase_parse(iter, end, parser, space, output) };

            THEN ("Parsing is successful")
            {
                const bool success { result && iter == end };
                REQUIRE(success);
            }
        }
    }
}

SCENARIO ("Entry with specified hour and singular retry time unit"
         " is parsed correctly", "[unit]")
{
    using parser_t = chronos::parser::parser;
    using boost::spirit::ascii::space;
    using chronos::parser::strct::TaskEntry;
    parser_t parser;

    TaskEntry output;

    GIVEN("Entry with 'at' part")
    {
        const std::string entry {
                "Run \"./program -i --param\" every day at 23:15"
                " retry after a minute;" };

        WHEN ("Entry is parsed")
        {
            std::string::const_iterator iter { entry.begin() };
            std::string::const_iterator end { entry.end() };
            const bool result {
                    phrase_parse(iter, end, parser, space, output) };

            THEN ("Parsing is successful")
            {
                const bool success { result && iter == end };
                REQUIRE(success);
            }
        }
    }
}

SCENARIO ("Entry is parsed correctly when minute part is 00", "[unit]")
{
    using parser_t = chronos::parser::parser;
    using boost::spirit::ascii::space;
    using chronos::parser::strct::TaskEntry;
    parser_t parser;

    TaskEntry output;

    GIVEN("The entry")
    {
        const std::string entry {
            "Run \"./program -i --param\" every day at 12:00;"};

        WHEN ("Entry is parsed")
        {
            std::string::const_iterator iter { entry.begin() };
            std::string::const_iterator end { entry.end() };
            const bool result {
                    phrase_parse(iter, end, parser, space, output) };

            THEN ("Parsing is successful")
            {
                const bool success { result && iter == end };
                REQUIRE(success);
            }
        }
    }
}

SCENARIO ("Closest time point for given week time is correct", "[unit]")
{
    using task_builder_t = chronos::TaskBuilder<test::artificial_clock_t>;
    using ptime_t = boost::posix_time::ptime;
    using date_t = boost::gregorian::date;
    task_builder_t task_builder;

    GIVEN ("A friday (7 August 2020)")
    {
        test::artificial_clock_t::time = ptime_t(
                date_t(2020, 8, 7));

        WHEN ("A task is created to be executed every monday")
        {
            const auto task {
                task_builder
                .everyWeeksCount(1)
                .atWeekDay({ .day = 1, .hour = 0, .minute = 0 })
                .build() };

            THEN ("Resulting task execution time is next monday")
            {
                const auto correct_execution_time { ptime_t(
                        date_t(2020, 8, 10)) };
                const auto result_execution_time { task.time };
                REQUIRE(result_execution_time == correct_execution_time);
            }
        }
    }
}

SCENARIO ("Closest time point is correct for week-related point "
          "calculated on sunday", "[unit]")
{
    using task_builder_t = chronos::TaskBuilder<test::artificial_clock_t>;
    using ptime_t = boost::posix_time::ptime;
    using date_t = boost::gregorian::date;
    task_builder_t task_builder;

    GIVEN ("A sunday (9 August 2020)")
    {
        test::artificial_clock_t::time = ptime_t(
                date_t(2020, 8, 9));

        WHEN ("A task is created to be executed every monday")
        {
            const auto task {
                task_builder
                .everyWeeksCount(1)
                .atWeekDay({ .day = 1, .hour = 0, .minute = 0 })
                .build() };

            THEN ("Resulting task execution time is next monday")
            {
                const auto correct_execution_time { ptime_t(
                        date_t(2020, 8, 10)) };
                const auto result_execution_time { task.time };
                REQUIRE(result_execution_time == correct_execution_time);
            }
        }
    }
}

SCENARIO ("Closest time point is correct for month-related point", "[unit]")
{
    using task_builder_t = chronos::TaskBuilder<test::artificial_clock_t>;
    using ptime_t = boost::posix_time::ptime;
    using date_t = boost::gregorian::date;
    using hours_t = boost::posix_time::hours;
    using minutes_t = boost::posix_time::minutes;
    task_builder_t task_builder;

    GIVEN ("A day in middle of month (17 August 2020) ")
    {
        test::artificial_clock_t::time = ptime_t(
                date_t(2020, 8, 17));

        WHEN ("A task is created to be executed "
              "every three months at day 16, 22:22")
        {
            const auto task {
                task_builder
                .everyMonthsCount(3)
                .atMonthDay({ .day = 16, .hour = 22, .minute = 22 })
                .build() };

            THEN ("Resulting task execution time"
                  " is 16 September 2020 22:22")
            {
                const auto correct_execution_time { ptime_t(
                        date_t(2020, 9, 16),
                        hours_t(22) + minutes_t(22)) };
                const auto result_execution_time { task.time };
                REQUIRE(correct_execution_time == result_execution_time);
            }
        }
    }
}

SCENARIO ("If task execution day is set to 31,"
          " correct day number persist through 30-day months",
          "[unit]")
{
    using task_builder_t = chronos::TaskBuilder<test::artificial_clock_t>;
    using ptime_t = boost::posix_time::ptime;
    using date_t = boost::gregorian::date;
    task_builder_t task_builder;

    GIVEN ("A first day of 31-day month (1 October 2020)")
    {
        test::artificial_clock_t::time = ptime_t(
                date_t(2020, 10, 1));

        WHEN ("A task is created to be executed every month at day 31"
              " and transitioned twice")
        {
            auto task {
                task_builder
                .everyMonthsCount(1)
                .atMonthDay({ .day = 31, .hour = 0, .minute = 0 })
                .build() };

            transit(task);
            transit(task);

            THEN ("Execution day is set to 31 December")
            {
                const auto correct_execution_time { ptime_t(
                        date_t(2020, 12, 31)) };
                const auto result_execution_time { task.time };
                REQUIRE (correct_execution_time == result_execution_time);
            }
        }
    }
}

SCENARIO ("If task execution day is set to 31,"
          " correct day number persist through February",
          "[unit]")
{
    using task_builder_t = chronos::TaskBuilder<test::artificial_clock_t>;
    using ptime_t = boost::posix_time::ptime;
    using date_t = boost::gregorian::date;
    task_builder_t task_builder;

    GIVEN ("A first day of January (1 January 2021)")
    {
        test::artificial_clock_t::time = ptime_t(
                date_t(2021, 1, 1));

        WHEN ("A task is created to be executed every month at day 31"
              "and transitioned twice")
        {
            auto task {
                task_builder
                .everyMonthsCount(1)
                .atMonthDay({ .day = 31, .hour = 0, .minute = 0 })
                .build() };

            transit(task);
            transit(task);

            THEN ("Execution day is set to 31 March 2021")
            {
                const auto correct_execution_time { ptime_t(
                        date_t(2021, 3, 31)) };
                const auto result_execution_time { task.time };
                REQUIRE (correct_execution_time == result_execution_time);
            }
        }
    }
}