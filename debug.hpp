#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>
#include <netinet/in.h>

std::ostream &operator<<(std::ostream &os, const sockaddr_in &address);

class Debug {
public:
    static Debug stream;

    struct endl_t {};
    static const endl_t endl;

    Debug(const Debug &) = delete;
    Debug &operator=(const Debug &) = delete;

    template<typename T>
    Debug &operator<<(const T &v) {
        if (m_enabled)
            std::cerr << v;
        return *this;
    }

    Debug &operator<<(const endl_t &) {
        if (m_enabled)
            std::cerr << std::endl;
        return *this;
    }

    void enable() {
        m_enabled = true;
    }

private:
    Debug() : m_enabled(false) {}

    bool m_enabled;
};

#endif // DEBUG_HPP
