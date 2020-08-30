#pragma once
#include <atomic>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include "fmt/core.h"

#if __GNUC__ > 7
#include <filesystem>
namespace std_filesystem = std::filesystem;
#else
#include <experimental/filesystem>
namespace std_filesystem = std::experimental::filesystem;
#endif


namespace chronos::filesystem::error
{
    class FileNotFound : public std::runtime_error
    {
    public:
        explicit FileNotFound(const std::string &path)
            : std::runtime_error(fmt::format(
                    "File not found : {}", path)) { }
    };
}

namespace chronos::filesystem::detail
{
    void check_if_file_exist(const std_filesystem::path &path)
    {
        if (!std_filesystem::exists(path))
            throw error::FileNotFound(path);
    }

    std::string read_file_content(const std_filesystem::path &path)
    {
        std::ifstream file;
        file.open(path);
        std::ostringstream string_stream;
        string_stream << file.rdbuf();
        std::string content { string_stream.str() };
        return content;
    }

    size_t calculate_file_hash(const std_filesystem::path &path)
    {
        check_if_file_exist(path);
        const auto content { read_file_content(path) };
        std::hash<std::string> hash;
        return hash(content);
    }
}

namespace chronos::filesystem::guard
{
    class FileGuard
    {
    public:
        explicit FileGuard(const std_filesystem::path &path)
            : path(path),
            previous_hash(detail::calculate_file_hash(path)) { }

        bool checkForChange()
        {
            const auto current_hash { detail::calculate_file_hash(path) };
            const bool changed { current_hash != previous_hash };
            previous_hash = current_hash;
            return changed;
        }

    private:
        std_filesystem::path path;
        size_t previous_hash;
    };
}

namespace chronos::filesystem::reader
{
    template <typename ParserT, typename ScheduleT>
    class FileReader
    {
    public:
        std::shared_ptr<ScheduleT> read(const std_filesystem::path &path)
        {
            filesystem::detail::check_if_file_exist(path);
            const auto content { filesystem::detail::read_file_content(path) };
            const auto schedule { parseContent(content) };
            return schedule;
        }

    private:
        std::shared_ptr<ScheduleT> parseContent(const std::string &content)
        {
            auto schedule { std::make_shared<ScheduleT>() };
            for (const auto &task : parser.parse(content))
                schedule->add(task);
            return schedule;
        }

        ParserT parser;
    };
}

namespace chronos
{
    template <typename TimerT>
    class FileLock
    {
    public:
        explicit FileLock(const std_filesystem::path &path) : guard(path) { }

        void waitUntilChange(const typename TimerT::duration_t &check_interval)
        {
            released = false;
            while (!guard.checkForChange() && !released)
                timer.wait(check_interval);
        }

        void release()
        {
            released = true;
            timer.interrupt();
        }

    private:
        TimerT timer;
        filesystem::guard::FileGuard guard;
        std::atomic<bool> released { false };
    };

    template <typename ParserT, typename ScheduleT>
    std::shared_ptr<ScheduleT>
    read_schedule_file(const std_filesystem::path &path)
    {
        filesystem::reader::FileReader<ParserT, ScheduleT> reader;
        return reader.read(path);
    }
}
