#pragma once
#include <filesystem>
#include <stdexcept>
#include "fmt/color.h"
#include "fmt/core.h"


namespace chronos::io::error
{
    class WrongNumberOfArguments : public std::runtime_error
    {
    public:
        explicit WrongNumberOfArguments(int args_count)
            : std::runtime_error(fmt::format(
                    "Invalid number of arguments: {}", args_count))
                    { }
    };
}

namespace chronos::io
{
    void validate_arguments_count(int argc)
    {
        constexpr auto CORRECT_ARGC { 2 };
        if (argc != CORRECT_ARGC)
            throw io::error::WrongNumberOfArguments(--argc);
    }
}

namespace chronos
{
    std::filesystem::path read_file_path_from_arg(int argc, char **argv)
    {
        io::validate_arguments_count(argc);
        constexpr auto SOURCE_FILE_ARG { 1 };
        const auto source_path { argv[SOURCE_FILE_ARG] };
        return std::filesystem::path(source_path);
    }

    void print_error_message(const std::string &message)
    {
        constexpr auto FOREGROUND_COLOR { fmt::color::crimson };
        constexpr auto TEXT_EMPHASIS { fmt::emphasis::bold };
        const auto error_message_formatting {
            fg(FOREGROUND_COLOR) | TEXT_EMPHASIS };
        fmt::print(error_message_formatting, message);
    }
}