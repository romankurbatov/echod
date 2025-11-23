#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <optional>
#include <netinet/in.h>

class Config {
public:
    Config(int argc, char *argv[]);

    Config(const Config &) = delete;
    void operator=(const Config &) = delete;

    bool is_valid() const { return m_valid; }
    bool need_help() const { return m_need_help; }
    bool debug() const { return m_debug; }

    const std::vector<sockaddr_in> &udp_addresses() const {
        return m_udp_addresses;
    }

    const std::vector<sockaddr_in> &tcp_addresses() const {
        return m_tcp_addresses;
    }

    static void show_help();

private:
    bool parse(int argc, char *argv[]);
    bool handle_udp_address(const char *s);
    bool handle_tcp_address(const char *s);

    std::optional<sockaddr_in> parse_address(const char *s);
    std::optional<in_addr> parse_ip_address(const char *s, const char *end);
    std::optional<in_port_t> parse_port(const char *s);

    bool m_need_help;
    bool m_valid;
    bool m_debug;

    std::vector<sockaddr_in> m_udp_addresses;
    std::vector<sockaddr_in> m_tcp_addresses;
};

#endif // CONFIG_HPP
