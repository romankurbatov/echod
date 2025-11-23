#include <iostream>
#include <stdlib.h>
#include <arpa/inet.h>

#include "config.hpp"

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

    std::cout << "UDP addresses: ";
    for (const sockaddr_in &addr : config.udp_addresses()) {
        char ip_addr_str[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &addr.sin_addr, ip_addr_str, sizeof(ip_addr_str))) {
            std::cout << "??? ";
            continue;
        }

        std::cout << ip_addr_str << ':' << addr.sin_port << ' ';
    }
    std::cout << std::endl;

    std::cout << "TCP addresses: ";
    for (const sockaddr_in &addr : config.tcp_addresses()) {
        char ip_addr_str[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &addr.sin_addr, ip_addr_str, sizeof(ip_addr_str))) {
            std::cout << "??? ";
            continue;
        }

        std::cout << ip_addr_str << ':' << addr.sin_port << ' ';
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}
