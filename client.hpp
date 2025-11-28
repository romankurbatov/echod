#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>

#include "listener.hpp"
#include "dispatcher.hpp"

class Client : public Listener {
public:
    Client(int fd, Dispatcher &dispatcher,
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
    int m_socket_fd;
    Dispatcher &m_dispatcher;
    const sockaddr_in m_client_address;
    const sockaddr_in m_server_address;
    char m_buffer[1024];
};

#endif // CLIENT_HPP
