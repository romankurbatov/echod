#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <stdint.h>

class Listener {
public:
    virtual void read_cb(uint32_t events) = 0;
    virtual ~Listener();
};

#endif // LISTENER_HPP
