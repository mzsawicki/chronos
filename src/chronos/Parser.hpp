#pragma once
#include <string>
#include <variant>
#include <boost/spirit/home/qi/string/symbols.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>


namespace chronos::parser::literals
{
    constexpr auto QUOTE { '"' };
    constexpr auto ENDL {';'};
    constexpr auto COLON { ':' };
    constexpr auto COMMA { '.' };

    constexpr auto A { "a" };
    constexpr auto AN { "an" };

    constexpr auto RUN { "run" };
    constexpr auto EVERY { "every" };
    constexpr auto AT { "at" };
    constexpr auto RETRY_AFTER { "retry after" };
    constexpr auto TIME { "time" };
    constexpr auto TIMES { "times" };
}

namespace chronos::parser::enums
{
    enum class TaskFrequency
    {
        MINUTES,
        HOURS,
        DAYS,
        WEEKS,
        MONTHS
    };

    enum class RetryTime
    {
        SECONDS,
        MINUTES,
        HOURS,
        DAYS
    };

    enum class WeekDay
    {
        MONDAY,
        TUESDAY,
        WEDNESDAY,
        THURSDAY,
        FRIDAY,
        SATURDAY,
        SUNDAY
    };
}

namespace chronos::parser::strct
{
    using namespace enums;

    struct TaskEntry
    {
        struct FrequencyPart
        {
            int frequency_time_count;
            TaskFrequency frequency_unit;
        };

        struct AtPart
        {
            using day_t = boost::variant<int, WeekDay>;

            day_t day;
            int hour;
            int minute;
        };

        struct RetryPart
        {
            int retry_time_count;
            RetryTime retry_time_unit;
            int retries_count;
        };

        std::string command;
        FrequencyPart frequency_part;
        AtPart at_part;
        RetryPart retry_part;
    };
}


BOOST_FUSION_ADAPT_STRUCT(
        chronos::parser::strct::TaskEntry::RetryPart,
        (int, retry_time_count),
        (chronos::parser::enums::RetryTime, retry_time_unit),
        (int, retries_count))

BOOST_FUSION_ADAPT_STRUCT(
        chronos::parser::strct::TaskEntry::FrequencyPart,
        (int, frequency_time_count),
        (chronos::parser::enums::TaskFrequency, frequency_unit))

BOOST_FUSION_ADAPT_STRUCT(
        chronos::parser::strct::TaskEntry::AtPart,
        (chronos::parser::strct::TaskEntry::AtPart::day_t, day),
        (int, hour),
        (int, minute))

BOOST_FUSION_ADAPT_STRUCT(
        chronos::parser::strct::TaskEntry,
        (std::string, command),
        (chronos::parser::strct::TaskEntry::FrequencyPart, frequency_part),
        (chronos::parser::strct::TaskEntry::AtPart, at_part)
        (chronos::parser::strct::TaskEntry::RetryPart, retry_part))


namespace chronos::parser::symbols
{
    using boost::spirit::qi::symbols;

    using namespace enums;

    struct task_frequency_unit_plural : symbols<char, TaskFrequency>
    {
        task_frequency_unit_plural()
        {
            add
                ("minutes", TaskFrequency::MINUTES)
                ("hours", TaskFrequency::HOURS)
                ("days", TaskFrequency::DAYS)
                ("weeks", TaskFrequency::WEEKS)
                ("months", TaskFrequency::MONTHS);
        }
    };

    struct task_frequency_unit_singular : symbols<char, TaskFrequency>
    {
        task_frequency_unit_singular()
        {
            add
                ("minute", TaskFrequency::MINUTES)
                ("hour", TaskFrequency::HOURS)
                ("day", TaskFrequency::DAYS)
                ("week", TaskFrequency::WEEKS)
                ("month", TaskFrequency::MONTHS);
        }
    };

    struct retry_frequency_unit_plural : symbols<char, RetryTime>
    {
        retry_frequency_unit_plural()
        {
            add
                ("seconds", RetryTime::SECONDS)
                ("minutes", RetryTime::MINUTES)
                ("hours", RetryTime::HOURS)
                ("days", RetryTime::DAYS);
        }
    };

    struct retry_frequency_unit_singular : symbols<char, RetryTime>
    {
        retry_frequency_unit_singular()
        {
            add
                ("second", RetryTime::SECONDS)
                ("minute", RetryTime::MINUTES)
                ("hour", RetryTime::HOURS)
                ("day", RetryTime::DAYS);
        }
    };

    struct week_day : symbols<char, WeekDay>
    {
        week_day()
        {
            add
                ("monday", WeekDay::MONDAY)
                ("tuesday", WeekDay::TUESDAY)
                ("wednesday", WeekDay::WEDNESDAY)
                ("thursday", WeekDay::THURSDAY)
                ("friday", WeekDay::FRIDAY)
                ("saturday", WeekDay::SATURDAY)
                ("sunday", WeekDay::SUNDAY);
        }
    };
}

namespace chronos::parser
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    using namespace enums;
    using namespace literals;
    using namespace strct;
    using namespace symbols;

    using iterator_t = std::string::const_iterator;
    using token_t = std::string;
    using space_t = ascii::space_type;

    using boost::spirit::ascii::no_case;
    using qi::attr;
    using qi::char_;
    using qi::grammar;
    using qi::lexeme;
    using qi::lit;
    using qi::omit;
    using qi::rule;
    using qi::uint_;

    struct parser : grammar<iterator_t, TaskEntry(), space_t>
    {
        using task_entry_rule = rule<iterator_t, TaskEntry(), space_t>;
        using retry_part_rule =
                rule<iterator_t, TaskEntry::RetryPart(), space_t>;
        using retry_times_rule = rule<iterator_t, int, space_t>;
        using frequency_rule =
                rule<iterator_t, TaskEntry::FrequencyPart(), space_t>;
        using at_part_rule = rule<iterator_t, TaskEntry::AtPart(), space_t>;

        rule<iterator_t, std::string(), space_t> article;
        rule<iterator_t, std::string(), space_t> time_s;
        rule<iterator_t, std::string(), space_t> command;
        frequency_rule frequency_plural;
        frequency_rule frequency_singular;
        at_part_rule at_day;
        at_part_rule at_hour;
        at_part_rule at_minute;
        at_part_rule at;
        at_part_rule at_placeholder;
        retry_part_rule retry;
        retry_part_rule retry_plural;
        retry_part_rule retry_singular;
        retry_part_rule retry_placeholder;
        retry_times_rule retry_times;
        retry_times_rule retry_times_placeholder;
        task_entry_rule start;
        task_frequency_unit_plural task_frequency_unit_plural_;
        task_frequency_unit_singular task_frequency_unit_singular_;
        retry_frequency_unit_plural retry_frequency_unit_plural_;
        retry_frequency_unit_singular retry_frequency_unit_singular_;
        week_day week_day_;

        parser() : parser::base_type(start)
        {
            article %= no_case[lit(AN)] | no_case[lit(A)];

            time_s %= no_case[lit(TIMES)] | no_case[lit(TIME)];

            command %= lexeme[QUOTE >> +(char_ - QUOTE) >> QUOTE];

            frequency_plural %= uint_ >> no_case[task_frequency_unit_plural_];

            frequency_singular %=
                    attr(1)
                    >> no_case[task_frequency_unit_singular_];

            at %=
                    no_case[lit(AT)]
                    >> (at_day | at_hour | at_minute);

            at_day %=
                    (uint_ | no_case[week_day_])
                    >> uint_
                    >> (omit[COLON] | omit[COMMA])
                    >> uint_;

            at_hour %=
                    attr(0)
                    >> uint_
                    >> (omit[COLON] | omit[COMMA])
                    >> uint_;

            at_minute %=
                    attr(0)
                    >> attr(0)
                    >> uint_;

            at_placeholder %= attr(0) >> attr(0) >> attr(0);

            retry_times %= uint_ >> time_s;

            retry_times_placeholder %= attr(1);

            retry_plural %=
                    uint_
                    >> no_case[retry_frequency_unit_plural_]
                    >> (retry_times | retry_times_placeholder);

            retry_singular %=
                    attr(1)
                    >> omit[-article]
                    >> no_case[retry_frequency_unit_singular_]
                    >> (retry_times | retry_times_placeholder);

            retry %=
                    no_case[lit(RETRY_AFTER)]
                    >> (retry_plural | retry_singular);

            retry_placeholder %=
                    attr(0)
                    >> attr(RetryTime::SECONDS)
                    >> attr(0);

            start %=
                    no_case[lit(RUN)]
                    >> command
                    >> no_case[lit(EVERY)]
                    >> (frequency_plural | frequency_singular)
                    >> (at | at_placeholder)
                    >> (retry | retry_placeholder)
                    >> ENDL;
        }
    };
}
