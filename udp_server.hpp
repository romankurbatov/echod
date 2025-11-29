#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP

#include "listener.hpp"

#include <stdexcept>
#include <netinet/in.h>

#include "dispatcher.hpp"
#include "command_executor.hpp"

class UDPServer : public Listener {
public:
    UDPServer(Dispatcher &dispatcher, CommandExecutor &executor,
            const sockaddr_in &address);
    ~UDPServer();

    UDPServer(const UDPServer &) = delete;
    UDPServer &operator=(const UDPServer &) = delete;

    void read_cb(uint32_t events) override;

    class Error : public std::runtime_error {
    public:
        // inherit all constructors
        using std::runtime_error::runtime_error;
    };

private:
    void send_response(const char *rsp, size_t len, const sockaddr_in &address);
    void process_command(const char *cmd, size_t len, const sockaddr_in &address);

    Dispatcher &m_dispatcher;
    CommandExecutor &m_executor;
    int m_socket_fd;
    char m_input_buf[10240];
    char m_output_buf[128]; // for command response only
    const sockaddr_in m_address;
};

#endif // UDP_SERVER_HPP
