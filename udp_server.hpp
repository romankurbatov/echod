#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP

#include "listener.hpp"

#include <stdexcept>
#include <netinet/in.h>

#include "dispatcher.hpp"

class UDPServer : public Listener {
public:
    UDPServer(Dispatcher &dispatcher, const sockaddr_in &address);
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
    Dispatcher &m_dispatcher;
    int m_socket_fd;
    char m_buffer[10240];
};

#endif // UDP_SERVER_HPP
