#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include "fmt/core.h"

namespace chronos::filesystem::error
{
    class FileNotFound : public std::runtime_error
    {
    public:
        explicit FileNotFound(const std::string& path)
            : std::runtime_error(
                    fmt::format("File not found : {}", path)) { }
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
}

namespace chronos
{
    template <typename ParserT, typename ScheduleT>
    class FileReader
    {
    public:
        using schedule_ptr_t = std::shared_ptr<ScheduleT>;

        schedule_ptr_t read(const std::filesystem::path &path)
        {
            filesystem::check_if_file_exist(path);
            const auto content { filesystem::read_file_content(path) };
            const auto schedule { parseContent(content) };
            return schedule;
        }

    private:
        schedule_ptr_t parseContent(const std::string &content)
        {
            const auto schedule { std::make_shared<ScheduleT>() };
            for (const auto &task : parser.parse(content))
                schedule->add(task);
            return schedule;
        }

        ParserT parser;
    };
}
