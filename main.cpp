#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "config.hpp"
#include "debug.hpp"

void debug_print_address(const sockaddr_in &address) {
    char ip_addr_str[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &address.sin_addr,
            ip_addr_str, sizeof(ip_addr_str)))
    {
        Debug::stream << "??? ";
        return;
    }

    Debug::stream << ip_addr_str << ':' << address.sin_port << ' ';
}

void debug_print_udp_addresses(const std::vector<sockaddr_in> &addresses) {
    Debug::stream << "Will listen UDP address(es): ";

    for (const sockaddr_in &address : addresses)
        debug_print_address(address);

    Debug::stream << Debug::endl;
}

void debug_print_tcp_addresses(const std::vector<sockaddr_in> &addresses) {
    Debug::stream << "Will listen TCP address(es): ";

    for (const sockaddr_in &address : addresses)
        debug_print_address(address);

    Debug::stream << Debug::endl;
}

void debug_print_addresses(const Config &config) {
    if (!config.udp_addresses().empty())
        debug_print_udp_addresses(config.udp_addresses());

    if (!config.tcp_addresses().empty())
        debug_print_tcp_addresses(config.tcp_addresses());
}

int main(int argc, char *argv[]) {
    Config config(argc, argv);

    if (!config.is_valid()) {
        Config::show_help();
        return EXIT_FAILURE;
    }

    if (config.need_help()) {
        Config::show_help();
        return EXIT_SUCCESS;
    }

    if (config.udp_addresses().empty() && config.tcp_addresses().empty()) {
        std::cerr << argv[0] << ": must specify at least one address"
                  << std::endl;
        Config::show_help();
        return EXIT_FAILURE;
    }

    if (config.debug())
        Debug::stream.enable();

    debug_print_addresses(config);

    return EXIT_SUCCESS;
}
