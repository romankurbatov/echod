#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include "listener.hpp"

#include <stdexcept>
#include <netinet/in.h>

#include "dispatcher.hpp"
#include "command_executor.hpp"
#include "client_registry.hpp"

class TCPServer : public Listener {
public:
    TCPServer(Dispatcher &dispatcher, CommandExecutor &executor,
            ClientRegistry &registry, const sockaddr_in &address);
    ~TCPServer();

    TCPServer(const TCPServer &) = delete;
    TCPServer &operator=(const TCPServer &) = delete;

    void read_cb(uint32_t events) override;

    class Error : public std::runtime_error {
    public:
        // inherit all constructors
        using std::runtime_error::runtime_error;
    };

private:
    Dispatcher &m_dispatcher;
    CommandExecutor &m_executor;
    ClientRegistry &m_registry;
    int m_socket_fd;
    const sockaddr_in m_address;
};

#endif // TCP_SERVER_HPP
