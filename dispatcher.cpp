#include "dispatcher.hpp"

#include <iostream>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "debug.hpp"

Dispatcher::Dispatcher() {
    m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (m_epoll_fd < 0) {
        std::ostringstream ss;
        ss << "epoll_create1() failed: " << strerror(errno);
        throw Error(ss.str());
    }
}

Dispatcher::~Dispatcher() {
    int ret = close(m_epoll_fd);
    if (ret != 0) {
        std::cerr << "close() failed: " << strerror(errno) << std::endl;
    }
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
    epoll_event event;
    int ret;

    while ((ret = epoll_wait(m_epoll_fd, &event, 1, -1)) > 0) {
        Debug::stream << "New event" << Debug::endl;
        Listener *listener = static_cast<Listener *>(event.data.ptr);
        listener->read_cb(event.events);
    }

    if (ret < 0) {
        std::ostringstream ss;
        ss << "epoll_wait failed: " << strerror(errno);
        throw Error(ss.str());
    }
}
