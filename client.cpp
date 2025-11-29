#include "client.hpp"

#include <iostream>
#include <string>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "command_executor.hpp"
#include "client_registry.hpp"
#include "debug.hpp"

Client::Client(int fd, Dispatcher &dispatcher,
        CommandExecutor &executor, ClientRegistry &registry,
        const sockaddr_in &client_address,
        const sockaddr_in &server_address) :
    m_socket_fd(fd),
    m_dispatcher(dispatcher),
    m_executor(executor),
    m_registry(registry),
    m_client_address(client_address),
    m_server_address(server_address),
    m_state(State::OUT),
    m_command_buf_len(0)
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
        m_registry.client_disconnected(this);
        return;
    }

    ssize_t nrecv = read(m_socket_fd, m_buffer, sizeof(m_buffer));
    if (nrecv < 0) {
        std::cerr << "Failed to read from TCP connection fd=" << m_socket_fd
                  << " with " << m_client_address <<
                  ": " << strerror(errno) << std::endl;
        return;
    } else if (nrecv == 0) {
        Debug::stream << "Client fd=" << m_socket_fd << ' ' << m_client_address
                      << " disconnected" << Debug::endl;
        m_registry.client_disconnected(this);
        return;
    }

    Debug::stream << "Received " << nrecv << " bytes from TCP client fd="
                  << m_socket_fd << ' '
                  << m_client_address << " -> "
                  << m_server_address << Debug::endl;

    bool ok = process_messages(m_buffer, nrecv);
    if (!ok) {
        // Some error -- disconnect
        m_registry.client_disconnected(this);
    }
}

bool Client::process_messages(const char *buf, size_t len) {
    // There are may be one or more messages in received data.
    // First and last ones may be incomplete

    const char *part_begin = buf;
    const char *data_end = buf + len;
    while (part_begin < data_end) {
        // Find end of current message (or message part).
        // May be NULL if current message is not yet received in full
        const char *part_end = static_cast<const char *>(
                memchr(part_begin, '\n', data_end - part_begin));
        bool full = (part_end != nullptr);
        if (full) {
            part_end++; // include '\n' into message
        } else {
            part_end = data_end;
        }

        size_t part_len = part_end - part_begin;

        if (m_state == State::SKIP_MSG)
            goto next_part;

        if (m_state == State::OUT) {
            m_state = ((*part_begin == '/') ?
                    State::IN_COMMAND :
                    State::IN_ECHO_MSG);
        }

        if (m_state == State::IN_ECHO_MSG) {
            bool ok = send_response(part_begin, part_len);
            if (!ok)
                return false;
        } else if (m_state == State::IN_COMMAND) {
            if (m_command_buf_len + part_len > m_command_buf.size()) {
                // Command too long
                m_state = State::SKIP_MSG;
                const char TOO_LONG_RSP[] = "Command too long\n";
                bool ok = send_response(TOO_LONG_RSP, sizeof(TOO_LONG_RSP) - 1);
                if (!ok)
                    return false;
            } else {
                // Copy part of command into command buffer
                memcpy(m_command_buf.data() + m_command_buf_len,
                        part_begin, part_len);
                m_command_buf_len += part_len;

                if (full) {
                    // Command received in full
                    std::string cmd(m_command_buf.data(), m_command_buf_len-1);
                    Debug::stream << "TCP client command: " << cmd << Debug::endl;
                    bool ok = handle_command(m_command_buf, m_command_buf_len);
                    if (!ok)
                        return false;
                    m_command_buf_len = 0;
                }
            }
        }

    next_part:
        part_begin += part_len;
        if (full)
            m_state = State::OUT;
    }

    return true;
}

bool Client::send_response(const char *buf, size_t len) {
    ssize_t nsent = write(m_socket_fd, buf, len);
    if (nsent < 0) {
        std::cerr << "Failed to write to TCP connection fd=" << m_socket_fd
                  << " with " << m_client_address
                  << ": " << strerror(errno) << std::endl;
        return false;
    } else if (static_cast<size_t>(nsent) != len) {
        std::cerr << "Partial write to TCP connection fd=" << m_socket_fd
                  << " with " << m_client_address << std::endl;
        return false;
    }

    Debug::stream << "Sent " << nsent << " bytes to TCP client fd="
                  << m_socket_fd << ' '
                  << m_server_address << " -> "
                  << m_client_address << Debug::endl;

    return true;
}

bool Client::handle_command(
        const CommandExecutor::command_buffer_t &cmd, size_t len)
{
    std::string rsp;
    CommandExecutor::Result result = m_executor.execute(cmd, len, rsp);
    switch (result) {
    case CommandExecutor::Result::OK:
    case CommandExecutor::Result::UNKNOWN_COMMAND:
        break;
    case CommandExecutor::Result::INVALID_COMMAND:
        std::cerr << "Client command processing internal error" << std::endl;
        break;
    case CommandExecutor::Result::SHUTDOWN:
        return false; // will cause disconnect
    }

    return send_response(rsp.data(), rsp.length());
}
