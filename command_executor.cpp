#include "command_executor.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <time.h>

#include "client_registry.hpp"

CommandExecutor::CommandExecutor() :
    m_registry(nullptr)
{
}

void CommandExecutor::set_registry(ClientRegistry *registry) {
    m_registry = registry;
}

CommandExecutor::Result CommandExecutor::execute(
        const command_buffer_t &cmd_buf, size_t cmd_len, std::string &rsp)
{
    if (cmd_len < 1) {
        std::cerr << "Command too short" << std::endl;
        rsp = "Internal server error\n";
        return Result::INVALID_COMMAND;
    }

    if (cmd_buf[0] != '/') {
        std::cerr << "Command not starting with '/'" << std::endl;
        rsp = "Internal server error\n";
        return Result::INVALID_COMMAND;
    }

    if (cmd_buf[cmd_len-1] == '\n')
        cmd_len--;

    std::string cmd_str(cmd_buf.data(), cmd_len);
    auto it = m_string_to_command.find(cmd_str);
    if (it == m_string_to_command.cend()) {
        rsp = "Unknown command\n";
        return Result::UNKNOWN_COMMAND;
    }

    Command cmd = it->second;
    switch (cmd) {
    case Command::TIME:
        execute_time_command(rsp);
        break;
    case Command::STATS:
        execute_stats_command(rsp);
        break;
    case Command::SHUTDOWN:
        rsp = "Good bye!";
        return Result::SHUTDOWN;
    }

    return Result::OK;
}

void CommandExecutor::execute_time_command(std::string &rsp) {
    time_t now = time(nullptr);
    tm now_broken;
    localtime_r(&now, &now_broken);

    std::ostringstream ss;
    ss << std::put_time(&now_broken, "%Y-%m-%d %H:%M:%S") << '\n';

    rsp = ss.str();
}

void CommandExecutor::execute_stats_command(std::string &rsp) {
    if (!m_registry) {
        std::cerr << "CommandExecutor: client registry not set" << std::endl;
        rsp = "Internal server error\n";
        return;
    }

    long connected = 0, total = 0;
    m_registry->get_stats(connected, total);

    std::ostringstream ss;
    ss << "Clients connected: " << connected << ", total: " << total << '\n';
    rsp = ss.str();
}
