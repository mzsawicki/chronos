#include <iostream>
#include "chronos/Chronos.hpp"


void print_interval(const chronos::time_duration_t &interval)
{
    std::cout << interval << std::endl;
}

void print_interval(const chronos::days_duration_t &interval)
{
    std::cout << interval << " day/s" << std::endl;
}

void print_interval(const chronos::weeks_duration_t &interval)
{
    std::cout << interval << " days" << std::endl;
}

void print_interval(const chronos::months_duration_t &interval)
{
    std::cout << interval.number_of_months() << " months\n";
}

void print_task(const chronos::Task &task)
{
    const auto print_interval_func {
        [] (const auto &interval) { print_interval(interval); } };

    std::cout << "Command: " << task.command << std::endl
    << "Next execution time: " << task.time << std::endl
    << "Interval: ";
    std::visit(print_interval_func, task.interval);
    std::cout << "Attempts: " << task.attempts_count << std::endl
    << "Max retries: " << task.max_retries_count << std::endl
    << "Retry after: " << task.retry_after << std::endl << std::endl;
}

int main()
{
    std::string input_path;
    std::cout << "File path:\n> ";
    std::cin >> input_path;
    std::filesystem::path path(input_path);
    std::ifstream file;
    file.open(path);
    std::ostringstream string_stream;
    string_stream << file.rdbuf();
    std::string content { string_stream.str() };
    chronos::parser_t parser;
    try {
        const auto tasks { parser.parse(content) };
        for (const auto &task : tasks)
            print_task(task);
    }
    catch (const chronos::parser::error::SyntaxError &error) {
        std::cout << error.what();
    }
    return 0;
}