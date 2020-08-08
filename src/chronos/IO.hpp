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
        if (argc != 2)
            throw io::error::WrongNumberOfArguments(argc - 1);
    }
}

namespace chronos
{
    std::filesystem::path read_file_path(int argc, char **argv)
    {
        io::validate_arguments_count(argc);
        return std::filesystem::path(argv[1]);
    }

    void show_error_message(const std::string &message)
    {
        const auto error_message_formatting {
            fmt::fg(fmt::color::crimson) | fmt::emphasis::bold };
        fmt::print(error_message_formatting, message);
    }
}