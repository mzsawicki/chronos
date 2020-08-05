#define CATCH_CONFIG_MAIN
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "catch2/catch.hpp"
#include "chronos/Chronos.hpp"
#include "chronos/Parser.hpp"
#include "TestUtils.hpp"


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
    auto schedule = chronos::createSchedule<chronos::schedule_t>();
    failing_dispatcher_t dispatcher(schedule);

    GIVEN("Task with 3 max retries")
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

SCENARIO("Entry with retry parameters is parsed correctly", "[unit]")
{
    using parser_t = chronos::parser::parser;
    using boost::spirit::ascii::space;
    using chronos::parser::strct::TaskEntry;
    parser_t parser;

    TaskEntry output;

    GIVEN("Entry with retry time and count")
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

SCENARIO("Entry with specified time is parsed correctly", "[unit]")
{
    using parser_t = chronos::parser::parser;
    using boost::spirit::ascii::space;
    using chronos::parser::strct::TaskEntry;
    parser_t parser;

    TaskEntry output;

    GIVEN("Entry with 'at' part")
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

SCENARIO("Entry with specified hour and singular retry time unit"
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

SCENARIO("Entry is parsed correctly when minute part is 00", "[unit]")
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