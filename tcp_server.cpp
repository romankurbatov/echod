#include "tcp_server.hpp"

#include <netinet/in.h>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "command_executor.hpp"
#include "debug.hpp"
#include "client.hpp"

TCPServer::TCPServer(Dispatcher &dispatcher,
        CommandExecutor &executor, const sockaddr_in &address) :
    m_dispatcher(dispatcher),
    m_executor(executor),
    m_address(address)
{
    m_socket_fd = socket(PF_INET,
            SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (m_socket_fd < 0) {
        std::ostringstream ss;
        ss << "socket(SOCK_STREAM) failed: " << strerror(errno);
        throw Error(ss.str());
    }

    int ret = bind(m_socket_fd,
            reinterpret_cast<const sockaddr *>(&address),
            sizeof(address));
    if (ret != 0) {
        std::ostringstream ss;
        ss << "bind() to " << address << " failed: " << strerror(errno);
        throw Error(ss.str());
    }

    ret = listen(m_socket_fd, 10);
    if (ret != 0) {
        std::ostringstream ss;
        ss << "listen() failed: " << strerror(errno);
        throw Error(ss.str());
    }

    Debug::stream << "TCP server opened socket fd=" << m_socket_fd
                  << " to listen " << address << Debug::endl;

    dispatcher.register_listener(m_socket_fd, *this);
}

TCPServer::~TCPServer() {
    m_dispatcher.deregister_listener(m_socket_fd);

    int ret = close(m_socket_fd);
    if (ret != 0) {
        std::cerr << "close() failed: " << strerror(errno) << std::endl;
        return;
    }

    Debug::stream << "TCP server closed socket fd=" << m_socket_fd
                  << ", address: " << m_address << Debug::endl;
}

void TCPServer::read_cb(uint32_t events) {
    if (events & EPOLLERR)
        throw Error("EPOLLERR");

    if (events & EPOLLHUP)
        throw Error("EPOLLHUP");

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept4(m_socket_fd,
            reinterpret_cast<sockaddr *>(&client_addr),
            &client_addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (client_fd < 0) {
        std::cerr << "TCP server: accept() failed: "
                  << strerror(errno) << std::endl;
        return;
    }

    if (client_addr_len != sizeof(client_addr)) {
        std::cerr << "TCP server: address size mismatch" << std::endl;
        close(client_fd);
        return;
    }

    Debug::stream << "New TCP connection "
                  << client_addr << " -> " << m_address
                  << Debug::endl;

    new Client(client_fd, m_dispatcher, m_executor,
            client_addr, m_address);
}
