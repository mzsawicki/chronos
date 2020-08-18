#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include "fmt/core.h"


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

namespace chronos::filesystem
{
    void check_if_file_exist(const std::filesystem::path &path)
    {
        if (!std::filesystem::exists(path))
            throw error::FileNotFound(path);
    }

    std::string read_file_content(const std::filesystem::path &path)
    {
        std::ifstream file;
        file.open(path);
        std::ostringstream string_stream;
        string_stream << file.rdbuf();
        std::string content { string_stream.str() };
        return content;
    }

    size_t calculate_file_hash(const std::filesystem::path &path)
    {
        check_if_file_exist(path);
        const auto content { read_file_content(path) };
        std::hash<std::string> hash;
        return hash(content);
    }
}

namespace chronos
{
    template <typename ParserT, typename ScheduleT>
    class FileReader
    {
    public:
        std::shared_ptr<ScheduleT> read(const std::filesystem::path &path)
        {
            filesystem::check_if_file_exist(path);
            const auto content { filesystem::read_file_content(path) };
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

    class FileGuard
    {
    public:
        explicit FileGuard(const std::filesystem::path &path)
            : path(path),
            previous_hash(filesystem::calculate_file_hash(path)) { }

        bool checkForChange()
        {
            const auto current_hash { filesystem::calculate_file_hash(path) };
            const bool changed { current_hash != previous_hash };
            previous_hash = current_hash;
            return changed;
        }

    private:
        std::filesystem::path path;
        size_t previous_hash;
    };
}
