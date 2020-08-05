#pragma once
#include "boost/date_time/posix_time/posix_time.hpp"
#include "fmt/core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"


namespace chronos
{
    void setup_file_logger()
    {
        auto logger = spdlog::daily_logger_mt("logger",
                                              "log/log.txt",
                                              0, 0);
        logger->set_pattern("[%c] (%l): %v");
        spdlog::set_default_logger(logger);
    }

    void setup_console_logger()
    {
        auto logger = spdlog::stdout_color_mt("console");
        logger->set_pattern("[%c] (%l): %v");
        spdlog::set_default_logger(logger);
    }
}

namespace chronos::logging
{
    void log(const std::string &message)
    {
        spdlog::info(message);
    }

    void log_error(const std::string &message)
    {
        spdlog::error(message);
    }
}

namespace chronos::logging::schedule
{
    template <typename TaskT>
    void log_added_task(const TaskT &task)
    {
        using namespace boost::posix_time;
        const auto task_time_string { to_simple_string(task.time) };
        const std::string message = fmt::format(
                    "Added new task to schedule: \"{}\" to be executed at: {}",
                    task.command, task_time_string);
        log(message);
    }

    template <typename TaskT>
    void log_rescheduled_task(const TaskT &task)
    {
        using namespace boost::posix_time;
        const auto task_time_string { to_simple_string(task.time) };
        const std::string message = fmt::format(
                "Rescheduled task: \"{}\" to be executed at: {}",
                task.command, task_time_string);
        log(message);
    }

    template <typename TaskT>
    void log_before_retry(const TaskT &task)
    {
        const std::string message = fmt::format(
                "Task will be retried: \"{}\"", task.command);
        log(message);
    }
}

namespace chronos::logging::system
{
    void log_before_command_execution(const std::string &command)
    {
        const std::string message = fmt::format(
                "Executing command: \"{}\"", command);
        log(message);
    }

    void log_after_successful_execution(const std::string &command)
    {
        const std::string message { "Execution succeed" };
        log(message);
    }

    void log_after_failed_execution(const std::string &command)
    {
        const std::string message { "Execution failed" };
        log_error(message);
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

        void reschedule(const typename WrapeeT::task_t &task)
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
        bool operator () (const std::string &command)
        {
            logging::system::log_before_command_execution(command);
            const bool result { wrapee(command) };
            if (result)
                logging::system::log_after_successful_execution(command);
            else
                logging::system::log_after_failed_execution(command);
            return result;
        }

    private:
        WrapeeT wrapee;
    };
}