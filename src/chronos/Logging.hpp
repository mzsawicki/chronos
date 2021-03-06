#pragma once
#include "boost/date_time/posix_time/posix_time.hpp"
#include "fmt/core.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"


namespace chronos::logging::formatters::detail
{
    template <typename ClockT>
    std::string get_time_string()
    {
        using namespace boost::posix_time;
        const auto clock_time { ClockT::local_time() };
        const auto time_string { to_simple_string(clock_time) };
        return time_string;
    }

    template <typename ClockT>
    class CustomTimeFormatterFlag : public spdlog::custom_flag_formatter
    {
    public:
        void format(const spdlog::details::log_msg &msg, const tm &tm_time,
                    spdlog::memory_buf_t &dest) override
        {
            const auto time_string { get_time_string<ClockT>() };
            dest.append(time_string.data(),
                        time_string.data() + time_string.size());
        }

        [[nodiscard]]
        std::unique_ptr<custom_flag_formatter> clone() const override
        {
            return spdlog::details::make_unique<
                    CustomTimeFormatterFlag<ClockT> >();
        }
    };
}

namespace chronos::logging::formatters
{
    template <typename ClockT>
    std::unique_ptr<spdlog::pattern_formatter> get_custom_time_formatter()
    {
        auto formatter { std::make_unique<spdlog::pattern_formatter>() };
        formatter
            ->add_flag<detail::CustomTimeFormatterFlag<ClockT> >('#')
            .set_pattern("[%#]: %v");
        return formatter;
    }
}

namespace chronos
{
    void setup_file_logger()
    {
        auto logger = spdlog::daily_logger_mt("logger",
                                              "log/log.txt",
                                              0, 0);
        logger->set_pattern("[%c]: %v");
        spdlog::flush_on(spdlog::level::info);
        spdlog::set_default_logger(logger);
    }

    template <typename ClockT>
    void setup_console_logger()
    {
        using namespace logging::formatters;
        auto logger { spdlog::stdout_color_mt("console") };
        auto formatter { get_custom_time_formatter<ClockT>() };
        logger->set_formatter(std::move(formatter));
        spdlog::set_default_logger(logger);
    }
}

namespace chronos::logging
{
    void log(const std::string &message)
    {
        spdlog::info(message);
    }
}

namespace chronos::logging::schedule::detail
{
    template <typename TaskT>
    std::string added_task_normal_message(const TaskT &task)
    {
        using namespace boost::posix_time;
        const auto task_time_string { to_simple_string(task.time) };
        return fmt::format(
                "Added new task to schedule: \"{}\" to be executed at: {}",
                task.command, task_time_string);
    }

    template <typename TaskT>
    std::string added_task_retry_message(const TaskT &task)
    {
        using namespace boost::posix_time;
        const auto task_time_string { to_simple_string(task.time) };
        return fmt::format(
                "Added task retry ({}/{}) to schedule: \"{}\""
                " to be executed at: {}",
                task.attempts_count, task.max_retries_count,
                task.command, task_time_string);
    }
}

namespace chronos::logging::schedule
{
    template <typename TaskT>
    void log_added_task(const TaskT &task)
    {
        const std::string message { !task.attempts_count ?
            detail::added_task_normal_message(task)
            : detail::added_task_retry_message(task) };
        log(message);
    }

    template <typename TaskT>
    void log_rescheduled_task(const TaskT &task)
    {
        using namespace boost::posix_time;
        const auto task_time_string { to_simple_string(task.time) };
        const std::string message { fmt::format(
                "Rescheduled task: \"{}\" to be executed at: {}",
                task.command, task_time_string) };
        log(message);
    }

    template <typename TaskT>
    void log_before_retry(const TaskT &task)
    {
        const auto retries_left {
            task.max_retries_count - task.attempts_count };
        const std::string message { fmt::format(
                "Task \"{}\" will be retried (retries left: {})."
                " Time to retry: {}",
                task.command, retries_left,
                boost::posix_time::to_simple_string(task.retry_after)) };
        log(message);
    }
}

namespace chronos::logging::system
{
    void log_before_command_execution(const std::string &command)
    {
        const std::string message { fmt::format(
                "Executing command: \"{}\"", command) };
        log(message);
    }

    void append_system_response(std::string &src,
                                const std::string &system_response)
    {
        src.append(fmt::format(
                "\nSystem message: {}", system_response));
    }

    void log_after_successful_execution(const std::string &command,
                                        const std::string &response_message)
    {
        std::string message { "Execution succeed" };
        if (!response_message.empty())
            append_system_response(message, response_message);
        log(message);
    }

    void log_after_failed_execution(const std::string &command,
                                    const std::string &response_message)
    {
        std::string message { "Execution failed" };
        if (!response_message.empty())
            append_system_response(message, response_message);
        log(message);
    }
}

namespace chronos::logging::parser
{
    void log_parsing_error()
    {
        std::string message { "Parsing source file failed" };
        log(message);
    }
}

namespace chronos::logging::dispatcher
{
    void log_reload()
    {
        std::string message { "Schedule has been reloaded" };
        log(message);
    }
}

namespace chronos
{
    template <typename WrapeeT>
    class ScheduleLoggingProxy
    {
    public:
        using task_t = typename WrapeeT::task_t;
        using duration_t = typename WrapeeT::duration_t;

        [[nodiscard]] bool isEmpty() const
        {
            return wrapee.isEmpty();
        }

        void add(const typename WrapeeT::task_t &task)
        {
            wrapee.add(task);
            logging::schedule::log_added_task(task);
        }

        void reschedule(typename WrapeeT::task_t &task)
        {
            wrapee.reschedule(task);
            logging::schedule::log_rescheduled_task(task);
        }

        void retry(const typename WrapeeT::task_t &task)
        {
            logging::schedule::log_before_retry(task);
            wrapee.retry(task);
        }

        [[nodiscard]] typename WrapeeT::duration_t timeToNextTask() const
        {
            return wrapee.timeToNextTask();
        }

        typename WrapeeT::task_t withdrawNextTask()
        {
            return wrapee.withdrawNextTask();
        }

    private:
        WrapeeT wrapee;
    };

    template <typename WrapeeT>
    class SystemCallLoggingProxy
    {
    public:
        using response_t = typename WrapeeT::response_t;

        response_t operator () (const std::string &command)
        {
            logging::system::log_before_command_execution(command);
            const auto response { wrapee(command) };
            if (response.success)
                logging::system::log_after_successful_execution(command,
                                                            response.message);
            else
                logging::system::log_after_failed_execution(command,
                                                            response.message);
            return response;
        }

    private:
        WrapeeT wrapee;
    };

    template <typename WrapeeT>
    class ParserLoggingProxy
    {
    public:
        typename WrapeeT::result_t parse(const std::string &input)
        {
            try {
                return wrapee.parse(input);
            } catch (const std::exception &error) {
                logging::parser::log_parsing_error();
                throw;
            }
        }
    private:
        WrapeeT wrapee;
    };

    template <typename WrapeeT>
    class DispatcherLoggingProxy
    {
    public:
        using schedule_ptr_t = typename WrapeeT::schedule_ptr_t;
        using time_duration_t = typename WrapeeT::time_duration_t;

        explicit DispatcherLoggingProxy(schedule_ptr_t schedule)
            : wrapee(schedule) { }

        time_duration_t timeToNextTask() const
        {
            return wrapee.timeToNextTask();
        }

        void handleNextTask()
        {
            wrapee.handleNextTask();
        }

        void reload(schedule_ptr_t new_schedule)
        {
            wrapee.reload(new_schedule);
            logging::dispatcher::log_reload();
        }

    private:
        WrapeeT wrapee;
    };
}