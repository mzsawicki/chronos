#pragma once
#include <bits/stdc++.h>


namespace chronos::system::error
{
    class PipeOpeningFailed : public std::runtime_error
    {
    public:
        PipeOpeningFailed() : std::runtime_error("Pipe opening failed") { }
    };
}

namespace chronos::system::pipe
{
    static constexpr auto MESSAGE_BUFFER_SIZE { 128 };
    static constexpr auto RETURN_CODE_OK { 0 };
    static constexpr auto PIPE_TYPE { "r" };

    using pipe_ptr_t = FILE*;

    void verify_pipe_opening(const pipe_ptr_t &pipe)
    {
        if (!pipe)
            throw error::PipeOpeningFailed();
    }

    std::string append_stderr_redirect(const std::string &command)
    {
        return command + " 2>&1";
    }

    pipe_ptr_t open_pipe(const std::string &command)
    {
        const auto command_with_redirect { append_stderr_redirect(command) };
        pipe_ptr_t pipe(popen(command_with_redirect.c_str(), PIPE_TYPE));
        verify_pipe_opening(pipe);
        return pipe;
    }

    class ReadPipe
    {
    public:
        using message_buffer_t = std::array<char, MESSAGE_BUFFER_SIZE>;
        using message_t = std::string;

        explicit ReadPipe(const std::string &command)
            : pipe(open_pipe(command)) { };

        message_t drain()
        {
            std::string output;
            message_buffer_t buffer;
            while (fgets(buffer.data(), buffer.size(), pipe))
                output.append(buffer.data());
            return output;
        }

        bool close()
        {
            return pclose(pipe) == RETURN_CODE_OK;
        }

    private:
        pipe_ptr_t pipe;
    };
}

namespace chronos::system
{
    struct Response
    {
        bool success;
        std::string message;
    };
}

namespace chronos
{
    struct SystemCall
    {
        using response_t = system::Response;

        response_t operator () (const std::string &command)
        {
            system::pipe::ReadPipe pipe(command);
            const auto message { pipe.drain() };
            const auto success { pipe.close() };
            return { .success = success, .message = message };
        }
    };
}
