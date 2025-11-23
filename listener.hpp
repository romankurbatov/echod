#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <stdint.h>

// Base class for classes listening a socket
class Listener {
public:
    // Callback to be called by Dispatcher
    // when there is data available to read
    virtual void read_cb(uint32_t events) = 0;

    virtual ~Listener() {}
};

#endif // LISTENER_HPP
