#pragma once
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "boost/spirit/home/qi/string/symbols.hpp"
#include "boost/fusion/include/adapt_struct.hpp"
#include "boost/spirit/include/phoenix.hpp"
#include "boost/spirit/include/qi.hpp"


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

    constexpr int week_day_to_number(const WeekDay &day)
    {
        int day_number { 0 };
        switch (day)
        {
            case WeekDay::MONDAY:
                day_number = 1;
                break;
            case WeekDay::TUESDAY:
                day_number = 2;
                break;
            case WeekDay::WEDNESDAY:
                day_number = 3;
                break;
            case WeekDay::THURSDAY:
                day_number = 4;
                break;
            case WeekDay::FRIDAY:
                day_number = 5;
                break;
            case WeekDay::SATURDAY:
                day_number = 6;
                break;
            case WeekDay::SUNDAY:
                day_number = 7;
                break;
        }
        return day_number;
    }
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

    using namespace qi;
    using namespace enums;
    using namespace literals;
    using namespace strct;
    using namespace symbols;

    using iterator_t = std::string::const_iterator;
    using token_t = std::string;
    using space_t = ascii::space_type;

    using boost::spirit::ascii::no_case;

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
        rule<iterator_t, int, space_t> minute;
        rule<iterator_t, int, space_t> hour;
        rule<iterator_t, int, space_t> month_day;
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

            minute %= uint_ [_pass = (_1 >= 0 && _1 < 60)];

            hour %= uint_ [_pass = (_1 >= 0 && _1 < 25)];

            month_day %= uint_ [_pass = (_1 > 0 && _1 < 32)];

            at %=
                    no_case[lit(AT)]
                    >> (at_day | at_hour | at_minute);

            at_day %=
                    (month_day | no_case[week_day_])
                    >> hour
                    >> (omit[COLON] | omit[COMMA])
                    >> minute;

            at_hour %=
                    attr(0)
                    >> hour
                    >> (omit[COLON] | omit[COMMA])
                    >> minute;

            at_minute %=
                    attr(0)
                    >> attr(0)
                    >> minute;

            at_placeholder %= attr(1) >> attr(0) >> attr(0);

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

namespace chronos::parser::conversions
{
    using namespace enums;

    int minutes_to_seconds(int count)
    {
        constexpr int SECONDS_IN_MINUTE { 60 };
        return count * SECONDS_IN_MINUTE;
    }

    int hours_to_seconds(int count)
    {
        constexpr int SECONDS_IN_HOUR { 60 * 60 };
        return count * SECONDS_IN_HOUR;
    }

    int days_to_seconds(int count)
    {
        constexpr int SECONDS_IN_DAY { 24 * 60 * 60 };
        return count * SECONDS_IN_DAY;
    }

    int to_seconds(const RetryTime &unit, int count)
    {
        int seconds;
        switch (unit)
        {
            case RetryTime::SECONDS:
                seconds = count;
                break;
            case RetryTime::MINUTES:
                seconds = minutes_to_seconds(count);
                break;
            case RetryTime::HOURS:
                seconds = hours_to_seconds(count);
                break;
            case RetryTime::DAYS:
                seconds = days_to_seconds(count);
                break;
        }
        return seconds;
    }
}

namespace chronos::parser
{
    template <typename TaskBuilderT>
    class Converter
    {
    public:
        typename TaskBuilderT::task_t convert(const strct::TaskEntry &output)
        {
            task_builder
                .createTask()
                .withCommand(output.command);

            convertExecutionInfo(output);
            convertRetryInfo(output);

            return task_builder.build();
        }

    private:
        void convertExecutionInfo(const strct::TaskEntry &parser_output)
        {
            using namespace enums;
            const auto frequency { parser_output.frequency_part };
            const auto frequency_unit { frequency.frequency_unit };
            switch (frequency_unit)
            {
                case TaskFrequency::MINUTES:
                    setTimeForMinutesFrequency(parser_output);
                    break;
                case TaskFrequency::HOURS:
                    setTimeForHoursFrequency(parser_output);
                    break;
                case TaskFrequency::DAYS:
                    setTimeForDaysFrequency(parser_output);
                    break;
                case TaskFrequency::WEEKS:
                    setTimeForWeeksFrequency(parser_output);
                    break;
                case TaskFrequency::MONTHS:
                    setTimeForMonthsFrequency(parser_output);
                    break;
            }
        }

        void convertRetryInfo(const strct::TaskEntry &parser_output)
        {
            const auto retry_info { parser_output.retry_part };
            const auto retries_count { retry_info.retries_count };
            const auto retry_time_count { retry_info.retry_time_count };
            const auto retry_time_unit { retry_info.retry_time_unit };
            const auto retry_after_seconds {
                conversions::to_seconds(retry_time_unit, retry_time_count) };

            task_builder
                .retryTimes(retries_count)
                .retryAfter(retry_after_seconds);
        }

        void setTimeForMinutesFrequency(const strct::TaskEntry &parser_output)
        {
            const auto frequency { parser_output.frequency_part };
            const auto frequency_count { frequency.frequency_time_count };
            task_builder
                .everyMinutesCount(frequency_count)
                .atMinute();
        }

        void setTimeForHoursFrequency(const strct::TaskEntry &parser_output)
        {
            const auto frequency { parser_output.frequency_part };
            const auto at { parser_output.at_part };
            const auto frequency_count { frequency.frequency_time_count };
            task_builder
                .everyHoursCount(frequency_count)
                .atMinute(at.minute);
        }

        void setTimeForDaysFrequency(const strct::TaskEntry &parser_output)
        {
            const auto frequency { parser_output.frequency_part };
            const auto frequency_count { frequency.frequency_time_count };
            const auto at { parser_output.at_part };
            task_builder
                .everyDaysCount(frequency_count)
                .atHour({ at.hour, at.minute });
        }

        void setTimeForWeeksFrequency(const strct::TaskEntry &parser_output)
        {
            const auto frequency { parser_output.frequency_part };
            const auto frequency_count { frequency.frequency_time_count };
            const auto at { parser_output.at_part };
            const auto week_day { boost::get<WeekDay>(at.day) };
            task_builder
                .everyWeeksCount(frequency_count)
                .atWeekDay(
                    { .day = week_day_to_number(week_day),
                      .hour = at.hour, .minute = at.minute });
        }

        void setTimeForMonthsFrequency(const strct::TaskEntry &parser_output)
        {
            const auto frequency { parser_output.frequency_part };
            const auto frequency_count { frequency.frequency_time_count };
            const auto at { parser_output.at_part };
            const auto month_day { boost::get<int>(at.day) };
            task_builder
                .everyMonthsCount(frequency_count)
                .atMonthDay(
                    { .day = month_day,
                      .hour = at.hour, .minute = at.minute });
        }

        TaskBuilderT task_builder;
    };
}

namespace chronos::parser::error
{
    class SyntaxError : public std::runtime_error
    {
    public:
        explicit SyntaxError(const std::string &bad_entry)
            : std::runtime_error(bad_entry) { }
    };
}

namespace chronos::parser
{
    void throw_parsing_error(const std::string::const_iterator &iter)
    {
        std::string bad_part { *iter };
        throw error::SyntaxError("Syntax error at: " + bad_part);
    }
}

namespace chronos
{
    template <typename TaskBuilderT>
    class Parser
    {
    public:
        using parsing_output_t = std::vector<parser::strct::TaskEntry>;
        using result_t = std::vector<typename TaskBuilderT::task_t>;

        result_t parse(const std::string &input)
        {
            const auto parsing_output { parseToStruct(input) };
            result_t result;
            for (const auto &output : parsing_output)
                result.push_back(converter.convert(output));
            return result;
        }

    private:
        parsing_output_t parseToStruct(const std::string &input)
        {
            using boost::spirit::ascii::space;

            std::vector<parser::strct::TaskEntry> output;
            std::string::const_iterator iter(input.begin());
            std::string::const_iterator end(input.end());

            while (iter != end)
                if(!phrase_parse(iter, end, parser, space, output))
                    parser::throw_parsing_error(iter);

            return output;
        }

        parser::parser parser;
        parser::Converter<TaskBuilderT> converter;
    };
}
