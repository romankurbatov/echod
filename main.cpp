#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "config.hpp"
#include "debug.hpp"
#include "dispatcher.hpp"
#include "udp_server.hpp"
#include "tcp_server.hpp"

void debug_print_udp_addresses(const std::vector<sockaddr_in> &addresses) {
    Debug::stream << "Will listen UDP address(es): ";

    for (const sockaddr_in &address : addresses)
        Debug::stream << address << ' ';

    Debug::stream << Debug::endl;
}

void debug_print_tcp_addresses(const std::vector<sockaddr_in> &addresses) {
    Debug::stream << "Will listen TCP address(es): ";

    for (const sockaddr_in &address : addresses)
        Debug::stream << address << ' ';

    Debug::stream << Debug::endl;
}

void debug_print_addresses(const Config &config) {
    if (!config.udp_addresses().empty())
        debug_print_udp_addresses(config.udp_addresses());

    if (!config.tcp_addresses().empty())
        debug_print_tcp_addresses(config.tcp_addresses());
}

bool run(const Config &config) {
    try {
        Dispatcher dispatcher;

        std::vector<std::unique_ptr<UDPServer>> udp_servers;
        for (const sockaddr_in &address : config.udp_addresses()) {
            udp_servers.emplace_back(
                    std::make_unique<UDPServer>(dispatcher, address));
        }

        std::vector<std::unique_ptr<TCPServer>> tcp_servers;
        for (const sockaddr_in &address : config.tcp_addresses()) {
            tcp_servers.emplace_back(
                    std::make_unique<TCPServer>(dispatcher, address));
        }

        dispatcher.run();
    } catch (const Dispatcher::Error &e) {
        std::cerr << "Dispatcher error: " << e.what() << std::endl;
        return false;
    } catch (const UDPServer::Error &e) {
        std::cerr << "UDP server error: " << e.what() << std::endl;
        return false;
    } catch (const TCPServer::Error &e) {
        std::cerr << "TCP server error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    Config config(argc, argv);

    if (!config.is_valid()) {
        std::cerr << std::endl;
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
        std::cerr << std::endl;
        Config::show_help();
        return EXIT_FAILURE;
    }

    if (config.debug())
        Debug::stream.enable();

    debug_print_addresses(config);

    return run(config) ? EXIT_SUCCESS : EXIT_FAILURE;
}
