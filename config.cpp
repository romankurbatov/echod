#include "config.hpp"

#include <iostream>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>

Config::Config(int argc, char *argv[]) :
        m_need_help(false),
        m_debug(false)
{
    m_valid = parse(argc, argv);
}

bool Config::parse(int argc, char *argv[]) {
    option long_options[]{
        { "help", no_argument, nullptr, 'h' },
        { "udp", required_argument, nullptr, 'u' },
        { "tcp", required_argument, nullptr, 't' },
        { "debug", no_argument, nullptr, 'd' },
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hu:t:d",
            long_options, nullptr)) != -1)
    {
        bool ok;
        switch (opt) {
        case 'h':
            m_need_help = true;
            break;

        case 'u':
            ok = handle_udp_address(optarg);
            if (!ok) {
                std::cerr << argv[0] << ": invalid UDP address: '" << optarg
                          << "'" << std::endl;
                return false;
            }
            break;

        case 't':
            ok = handle_tcp_address(optarg);
            if (!ok) {
                std::cerr << argv[0] << ": invalid TCP address: '" << optarg
                          << "'" << std::endl;
                return false;
            }
            break;

        case 'd':
            m_debug = true;
            break;

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

bool Config::handle_udp_address(const char *s) {
    std::optional<sockaddr_in> address = parse_address(s);
    if (!address)
        return false;

    m_udp_addresses.push_back(*address);
    return true;
}

bool Config::handle_tcp_address(const char *s) {
    std::optional<sockaddr_in> address = parse_address(s);
    if (!address)
        return false;

    m_tcp_addresses.push_back(*address);
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

    std::optional<in_addr> ip_addr = parse_ip_address(s, delim);
    if (!ip_addr)
        return {};
    addr.sin_addr = *ip_addr;

    std::optional<in_port_t> port = parse_port(delim + 1);
    if (!port)
        return {};
    addr.sin_port = *port;

    return addr;
}

std::optional<in_addr> Config::parse_ip_address(const char *s, const char *end)
{
    char addr_str[INET_ADDRSTRLEN];
    size_t addr_len = end - s;
    if (addr_len > INET_ADDRSTRLEN - 1)
        return {};

    memcpy(addr_str, s, addr_len);
    addr_str[addr_len] = '\0';

    in_addr addr;
    int ret = inet_pton(AF_INET, addr_str, &addr);
    if (ret != 1)
        return {};

    return addr;
}

std::optional<in_port_t> Config::parse_port(const char *s) {
    if (!isdigit(s[0]))
        return {};

    char *end;
    long port = strtol(s, &end, 10);
    if (*end != '\0' || port > 0xffff)
        return {};

    return htons(port);
}

void Config::show_help() {
    std::cerr << "echod -- echo server with several commands" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Parameters:" << std::endl;
    std::cerr << "-h|--help -- print this message" << std::endl;
    std::cerr << "-u|--udp <address> -- UDP address to listen" << std::endl;
    std::cerr << "-t|--tcp <address> -- TCP address to listen" << std::endl;
    std::cerr << "-d|--debug -- enable debug output" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Address must be given in format: <IPv4-address>:<port>," << std::endl;
    std::cerr << "for example: 127.0.0.1:13579" << std::endl;
    std::cerr << "Multiple TCP/UDP addresses may be specified" << std::endl;
}
