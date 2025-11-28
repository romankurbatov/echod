#include "debug.hpp"

#include <sstream>
#include <arpa/inet.h>

std::ostream &operator<<(std::ostream &os, const sockaddr_in &address) {
    char ip_addr_str[INET_ADDRSTRLEN];
    bool ok = inet_ntop(AF_INET, &address.sin_addr,
            ip_addr_str, sizeof(ip_addr_str));
    if (!ok) {
        os.setstate(std::ios_base::failbit);
        return os;
    }

    std::ostringstream ss;
    ss << ip_addr_str << ':' << ntohs(address.sin_port);

    return os << ss.str();
}

Debug Debug::stream;
const Debug::endl_t Debug::endl;
