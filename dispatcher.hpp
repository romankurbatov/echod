#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <stdexcept>

#include "listener.hpp"

class Dispatcher {
public:
    Dispatcher();

    Dispatcher(const Dispatcher &) = delete;
    void operator=(const Dispatcher &) = delete;

    void register_listener(int fd, Listener &listener);
    void deregister_listener(int fd);

    void run();

    class Error : public std::runtime_error {
    public:
        // inherit all constructors
        using std::runtime_error::runtime_error;
    };

private:
    int m_epoll_fd;
};

#endif // DISPATCHER_HPP
