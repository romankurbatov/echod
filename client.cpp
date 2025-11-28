#include "client.hpp"

#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "debug.hpp"

Client::Client(int fd, Dispatcher &dispatcher,
        const sockaddr_in &client_address,
        const sockaddr_in &server_address) :
    m_socket_fd(fd),
    m_dispatcher(dispatcher),
    m_client_address(client_address),
    m_server_address(server_address)
{
    dispatcher.register_listener(fd, *this);
}

Client::~Client() {
    m_dispatcher.deregister_listener(m_socket_fd);

    int ret = close(m_socket_fd);
    if (ret != 0) {
        std::cerr << "Client: close() failed: "
                  << strerror(errno) << std::endl;
        return;
    }

    Debug::stream << "Closed TCP connection fd=" << m_socket_fd
                  << ' ' << m_client_address << " -> " << m_server_address
                  << Debug::endl;
}

void Client::read_cb(uint32_t events) {
    if (events & (EPOLLERR | EPOLLHUP)) {
        Debug::stream << "Error/hangup on TCP connection fd=" << m_socket_fd
                      << ' ' << m_client_address << " -> " << m_server_address
                      << Debug::endl;
        delete this;
        return;
    }

    ssize_t nrecv = read(m_socket_fd, m_buffer, sizeof(m_buffer));
    if (nrecv < 0) {
        std::cerr << "Failed to read from TCP connection fd=" << m_socket_fd
                  << " with " << m_client_address <<
                  ": " << strerror(errno) << std::endl;
        return;
    } else if (nrecv == 0) {
        std::cerr << "Client fd=" << m_socket_fd << ' ' << m_client_address
                  << " disconnected" << std::endl;
        delete this;
        return;
    }

    Debug::stream << "Received " << nrecv << " bytes from TCP client fd="
                  << m_socket_fd << ' '
                  << m_client_address << " -> "
                  << m_server_address << Debug::endl;

    ssize_t nsent = write(m_socket_fd, m_buffer, nrecv);
    if (nsent < 0) {
        std::cerr << "Failed to write to TCP connection fd=" << m_socket_fd
                  << " with " << m_client_address
                  << ": " << strerror(errno) << std::endl;
        delete this;
        return;
    } else if (nsent != nrecv) {
        std::cerr << "Partial write to TCP connection fd=" << m_socket_fd
                  << " with " << m_client_address << std::endl;
        delete this;
        return;
    }

    Debug::stream << "Sent " << nsent << " bytes to TCP client fd="
                  << m_socket_fd << ' '
                  << m_server_address << " -> "
                  << m_client_address << Debug::endl;
}
