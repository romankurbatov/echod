#include "config.hpp"

#include <iostream>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>

Config::Config(int argc, char *argv[]) :
        m_need_help(false)
{
    m_valid = parse(argc, argv);
}

bool Config::parse(int argc, char *argv[]) {
    option long_options[]{
        { "help", no_argument, nullptr, 'h' },
        { "udp", required_argument, nullptr, 'u' },
        { "tcp", required_argument, nullptr, 't' },
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hu:t:", long_options, nullptr)) != -1) {
        switch (opt) {
        case 'h':
            m_need_help = true;
            break;

        case 'u': {
            std::optional<sockaddr_in> address = parse_address(optarg);
            if (address) {
                m_udp_addresses.push_back(*address);
            } else {
                std::cerr << argv[0] << ": invalid UDP address: '" << optarg
                          << "'" << std::endl;
                return false;
            }
            break;
        }

        case 't': {
            std::optional<sockaddr_in> address = parse_address(optarg);
            if (address) {
                m_tcp_addresses.push_back(*address);
            } else {
                std::cerr << argv[0] << ": invalid TCP address: '" << optarg
                          << "'" << std::endl;
                return false;
            }
            break;
        }

        default:
            return false;
        }
    }

    if (optind < argc) {
        std::cerr << argv[0] << ": " << "extra argument in command line"
                  << std::endl;
        return false;
    }

    return true;
}

std::optional<sockaddr_in> Config::parse_address(const char *s) {
    // Delimiter of IPv4 address and TCP/UDP port
    const char *delim = strchr(s, ':');
    if (!delim)
        return {};

    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;

    char ip_addr_str[INET_ADDRSTRLEN];
    size_t ip_addr_len = delim - s;
    if (ip_addr_len > INET_ADDRSTRLEN - 1)
        return {};

    memcpy(ip_addr_str, s, ip_addr_len);
    ip_addr_str[ip_addr_len] = '\0';

    int ret = inet_pton(AF_INET, ip_addr_str, &addr.sin_addr);
    if (ret != 1)
        return {};

    const char *port_str = delim + 1;
    if (!isdigit(port_str[0]))
        return {};

    char *end;
    long port = strtol(port_str, &end, 10);
    if (*end != '\0' || port > 0xffff)
        return {};

    addr.sin_port = port;

    return addr;
}

void Config::show_help() {
    std::cerr << "echod -- echo server with several commands" << std::endl;
    std::cerr << "-h|--help -- print this message" << std::endl;
    std::cerr << "-u|--udp <address> -- UDP address to listen" << std::endl;
    std::cerr << "-t|--tcp <address> -- TCP address to listen" << std::endl;
}
