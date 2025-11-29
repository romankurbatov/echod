#ifndef COMMAND_EXECUTOR_HPP
#define COMMAND_EXECUTOR_HPP

#include <array>
#include <string>
#include <unordered_map>
#include <stddef.h>

class ClientRegistry;

class CommandExecutor {
public:
    CommandExecutor();

    CommandExecutor(const CommandExecutor &) = delete;
    CommandExecutor &operator=(const CommandExecutor &) = delete;

    void set_registry(ClientRegistry *registry);

    static const size_t max_command_len = 16;
    using command_buffer_t = std::array<char, max_command_len>;

    enum class Result {
        OK,
        INVALID_COMMAND, // too short or not start with '/'
        UNKNOWN_COMMAND,
        SHUTDOWN,
    };

    Result execute(const command_buffer_t &cmd, size_t cmd_len,
            std::string &rsp);

private:
    ClientRegistry *m_registry;

    enum class Command {
        TIME,
        STATS,
        SHUTDOWN,
    };

    const std::unordered_map<std::string, Command> m_string_to_command{
        {"/time", Command::TIME},
        {"/stats", Command::STATS},
        {"/shutdown", Command::SHUTDOWN},
    };

    void execute_time_command(std::string &rsp);
    void execute_stats_command(std::string &rsp);
};

#endif // COMMAND_EXECUTOR_HPP
