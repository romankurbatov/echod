#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>

#include "listener.hpp"
#include "dispatcher.hpp"
#include "command_executor.hpp"

class ClientRegistry;

class Client : public Listener {
public:
    Client(int fd, Dispatcher &dispatcher,
            CommandExecutor &executor, ClientRegistry &registry,
            const sockaddr_in &client_address,
            const sockaddr_in &server_address);

    ~Client();

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    void read_cb(uint32_t events) override;

    class Error : public std::runtime_error {
    public:
        // inherit all constructors
        using std::runtime_error::runtime_error;
    };

private:
    bool process_messages(const char *buf, size_t len);
    bool handle_command(
            const CommandExecutor::command_buffer_t &cmd, size_t len);
    bool send_response(const char *buf, size_t len);

    int m_socket_fd;
    Dispatcher &m_dispatcher;
    CommandExecutor &m_executor;
    ClientRegistry &m_registry;
    const sockaddr_in m_client_address;
    const sockaddr_in m_server_address;
    char m_buffer[1024];

    enum class State {
        // Wait for new message
        OUT,

        // Wait for continuation of current message:

        // Current message is an ordinal echo message
        IN_ECHO_MSG,
        // Current message is a command
        IN_COMMAND,
        // Current message is a bad (too long) command, skip it
        SKIP_MSG,
    };

    State m_state;

    CommandExecutor::command_buffer_t m_command_buf;
    size_t m_command_buf_len;
};

#endif // CLIENT_HPP
