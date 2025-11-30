#include "dispatcher.hpp"

#include <iostream>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "debug.hpp"
#include "client_registry.hpp"

Dispatcher::Dispatcher() :
    m_registry(nullptr)
{
    m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (m_epoll_fd < 0) {
        std::ostringstream ss;
        ss << "epoll_create1() failed: " << strerror(errno);
        throw Error(ss.str());
    }

    Debug::stream << "Dispatcher opened epoll fd="
                  << m_epoll_fd << Debug::endl;
}

Dispatcher::~Dispatcher() {
    int ret = close(m_epoll_fd);
    if (ret != 0) {
        std::cerr << "close() failed: " << strerror(errno) << std::endl;
    }

    Debug::stream << "Dispatcher closed epoll fd="
                  << m_epoll_fd << Debug::endl;
}

void Dispatcher::set_registry(ClientRegistry *registry) {
    m_registry = registry;
}

void Dispatcher::register_listener(int fd, Listener &listener) {
    epoll_event event{
        .events = EPOLLIN,
        .data = {
            .ptr = &listener
        }
    };

    int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret != 0) {
        std::ostringstream ss;
        ss << "epoll_ctl(EPOLL_CTL_ADD) failed: " << strerror(errno);
        throw Error(ss.str());
    }

    Debug::stream << "Registered listener for fd=" << fd << Debug::endl;
}

void Dispatcher::deregister_listener(int fd) {
    int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    if (ret != 0) {
        std::ostringstream ss;
        ss << "epoll_ctl(EPOLL_CTL_DEL) failed: " << strerror(errno);
        throw Error(ss.str());
    }

    Debug::stream << "Deregistered listener for fd=" << fd << Debug::endl;
}

void Dispatcher::run() {
    sigset_t mask;
    sigemptyset(&mask);

    for (;;) {
        if (m_registry)
            m_registry->clear_disconnected_clients();

        epoll_event events[16];
        int nevents = epoll_pwait(m_epoll_fd,
                events, std::size(events), -1, &mask);
        if (nevents < 0) {
            if (errno == EINTR) {
                Debug::stream << "Exiting main event loop" << Debug::endl;
                break;
            }

            std::ostringstream ss;
            ss << "epoll_pwait failed: " << strerror(errno);
            throw Error(ss.str());
        }

        for (int i = 0; i < nevents; i++) {
            Debug::stream << "Dispatcher: new event" << Debug::endl;
            Listener *listener = static_cast<Listener *>(events[i].data.ptr);
            listener->read_cb(events[i].events);
        }
    }
}
