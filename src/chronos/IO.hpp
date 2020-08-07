#pragma once
#include <filesystem>
#include <stdexcept>
#include "fmt/core.h"


namespace chronos::io::error
{
    class WrongNumberOfArguments : public std::runtime_error
    {
    public:
        WrongNumberOfArguments(int args_count)
            : std::runtime_error(fmt::format(
                    "Invalid number of arguments: {}", args_count))
                    { }
    };
}

namespace chronos::io
{
    void validateArgumentsCount(int argc)
    {
        if (argc != 2)
            throw io::error::WrongNumberOfArguments(argc - 1);
    }
}

namespace chronos
{
    std::filesystem::path readChronosFilePath(int argc, char **argv)
    {
        io::validateArgumentsCount(argc);
        return std::filesystem::path(argv[0]);
    }
}