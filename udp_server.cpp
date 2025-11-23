#include "udp_server.hpp"

#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "dispatcher.hpp"
#include "debug.hpp"

UDPServer::UDPServer(Dispatcher &dispatcher, const sockaddr_in &address) :
        m_dispatcher(dispatcher)
{
    m_socket_fd = socket(PF_INET,
            SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (m_socket_fd < 0) {
        std::ostringstream ss;
        ss << "socket(SOCK_DGRAM) failed: " << strerror(errno);
        throw Error(ss.str());
    }

    int ret = bind(m_socket_fd, reinterpret_cast<const sockaddr *>(&address),
            sizeof(address));
    if (ret != 0) {
        std::ostringstream ss;
        ss << "bind() failed: " << strerror(errno);
        throw Error(ss.str());
    }

    dispatcher.register_listener(m_socket_fd, *this);
}

UDPServer::~UDPServer() {
    int ret = close(m_socket_fd);
    if (ret != 0) {
        std::cerr << "close() failed: " << strerror(errno) << std::endl;
    }
}

void UDPServer::read_cb(uint32_t events) {
    if (events & EPOLLERR)
        throw Error("EPOLLERR");

    if (events & EPOLLHUP)
        throw Error("EPOLLHUP");

    sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    ssize_t nrecv = recvfrom(m_socket_fd, m_buffer, sizeof(m_buffer), 0,
            reinterpret_cast<sockaddr *>(&src_addr), &src_addr_len);
    if (nrecv < 0) {
        std::ostringstream ss;
        ss << "recvfrom() failed: " << strerror(errno);
        throw Error(ss.str());
    }

    if (src_addr_len != sizeof(src_addr)) {
        std::ostringstream ss;
        ss << "src address size mismatch" << strerror(errno);
        throw Error(ss.str());
    }

    Debug::stream << "Received UDP datagram, length: " << nrecv << Debug::endl;

    ssize_t nsent = sendto(m_socket_fd, m_buffer, nrecv, 0,
            reinterpret_cast<const sockaddr *>(&src_addr), sizeof(src_addr));
    if (nsent < 0) {
        std::ostringstream ss;
        ss << "sendto() failed: " << strerror(errno);
        throw Error(ss.str());
    } else if (nsent < nrecv) {
        throw Error("Partial send");
    }

    Debug::stream << "Sent UDP datagram, length: " << nrecv << Debug::endl;
}
