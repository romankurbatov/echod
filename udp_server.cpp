#include "udp_server.hpp"

#include <iostream>
#include <iomanip>
#include <netinet/in.h>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "dispatcher.hpp"
#include "debug.hpp"

UDPServer::UDPServer(Dispatcher &dispatcher, const sockaddr_in &address) :
        m_dispatcher(dispatcher),
        m_address(address)
{
    m_socket_fd = socket(PF_INET,
            SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (m_socket_fd < 0) {
        std::ostringstream ss;
        ss << "socket(SOCK_DGRAM) failed: " << strerror(errno);
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

    Debug::stream << "UDP server opened socket fd=" << m_socket_fd
                  << " to listen " << address << Debug::endl;

    dispatcher.register_listener(m_socket_fd, *this);
}

UDPServer::~UDPServer() {
    m_dispatcher.deregister_listener(m_socket_fd);

    int ret = close(m_socket_fd);
    if (ret != 0) {
        std::cerr << "close() failed: " << strerror(errno) << std::endl;
        return;
    }

    Debug::stream << "UDP server closed socket fd=" << m_socket_fd
                  << ", address: " << m_address << Debug::endl;
}

void UDPServer::read_cb(uint32_t events) {
    if (events & EPOLLERR)
        throw Error("EPOLLERR");

    if (events & EPOLLHUP)
        throw Error("EPOLLHUP");

    sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    ssize_t nrecv = recvfrom(m_socket_fd, m_input_buf, sizeof(m_input_buf), 0,
            reinterpret_cast<sockaddr *>(&src_addr), &src_addr_len);
    if (nrecv < 0) {
        std::cerr << "UDP server: recvfrom() failed: "
                  << strerror(errno) << std::endl;
        return;
    }

    if (src_addr_len != sizeof(src_addr)) {
        std::cerr << "UDP server: address size mismatch" << std::endl;
        return;
    }

    Debug::stream << "Received UDP datagram "
                  << src_addr << " -> " << m_address
                  << ", length: " << nrecv << Debug::endl;

    if (nrecv == 0)
        return;

    if (m_input_buf[0] == '/') {
        process_command(m_input_buf + 1, nrecv - 1, src_addr);
    } else {
        send_response(m_input_buf, nrecv, src_addr);
    }
}

void UDPServer::send_response(
        const char *rsp, size_t len, const sockaddr_in &address)
{
    ssize_t nsent = sendto(m_socket_fd, rsp, len, 0,
            reinterpret_cast<const sockaddr *>(&address),
            sizeof(address));
    if (nsent < 0) {
        std::cerr << "UDP server: sendto() failed: "
                  << strerror(errno) << std::endl;
        return;
    } else if (static_cast<size_t>(nsent) < len) {
        std::cerr << "UDP server: partial send" << std::endl;
    }

    Debug::stream << "Sent UDP datagram "
                  << m_address << " -> " << address
                  << ", length: " << nsent << Debug::endl;
}

void UDPServer::process_command(
        const char *cmd, size_t len, const sockaddr_in &address)
{
    static const char TIME_CMD[] = "time\n";
    if (len == sizeof(TIME_CMD)-1 && memcmp(cmd, TIME_CMD, len) == 0) {
        time_t now = time(nullptr);
        tm now_broken;
        localtime_r(&now, &now_broken);

        std::ostringstream ss;
        ss << std::put_time(&now_broken, "%Y-%m-%d %H:%M:%S") << '\n';

        std::string now_str = ss.str();
        send_response(now_str.data(), now_str.length(), address);
    } else {
        std::cerr << "Unknown command from " << address << std::endl;
        send_response("?\n", 2, address);
    }
}
