#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.hpp"
#include "debug.hpp"
#include "dispatcher.hpp"
#include "udp_server.hpp"
#include "tcp_server.hpp"
#include "command_executor.hpp"
#include "client_registry.hpp"

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
        CommandExecutor executor;
        ClientRegistry registry(dispatcher, executor);

        dispatcher.set_registry(&registry);
        executor.set_registry(&registry);

        std::vector<std::unique_ptr<UDPServer>> udp_servers;
        for (const sockaddr_in &address : config.udp_addresses()) {
            udp_servers.emplace_back(
                    std::make_unique<UDPServer>(
                            dispatcher, executor, address));
        }

        std::vector<std::unique_ptr<TCPServer>> tcp_servers;
        for (const sockaddr_in &address : config.tcp_addresses()) {
            tcp_servers.emplace_back(
                    std::make_unique<TCPServer>(
                            dispatcher, executor, registry, address));
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

void handle_signal(int) {
}

bool init_signals() {
    // Mask SIGINT and SIGTERM. They will be unmasked
    // while process will be blocked on epoll_pwait()
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    int ret = sigprocmask(SIG_SETMASK, &mask, nullptr);
    if (ret != 0) {
        std::cerr << "sigprocmask() failed: "
                  << strerror(errno) << std::endl;
        return false;
    }

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = &handle_signal;
    sigemptyset(&sa.sa_mask);

    ret = sigaction(SIGINT, &sa, nullptr);
    if (ret != 0) {
        std::cerr << "sigaction(SIGINT) failed: "
                  << strerror(errno) << std::endl;
        return false;
    }

    ret = sigaction(SIGTERM, &sa, nullptr);
    if (ret != 0) {
        std::cerr << "sigaction(SIGTERM) failed: "
                  << strerror(errno) << std::endl;
        return false;
    }

    // Ignore SIGPIPE to prevent program termination
    // on write to connection which is closed by peer
    signal(SIGPIPE, SIG_IGN);

    return true;
}

int main(int argc, char *argv[]) {
    bool ok = init_signals();
    if (!ok)
        return EXIT_FAILURE;

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
