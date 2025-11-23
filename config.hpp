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

    const std::vector<sockaddr_in> &udp_addresses() const {
        return m_udp_addresses;
    }

    const std::vector<sockaddr_in> &tcp_addresses() const {
        return m_tcp_addresses;
    }

    static void show_help();

private:
    bool parse(int argc, char *argv[]);
    std::optional<sockaddr_in> parse_address(const char *s);

    bool m_need_help;
    bool m_valid;

    std::vector<sockaddr_in> m_udp_addresses;
    std::vector<sockaddr_in> m_tcp_addresses;
};

#endif // CONFIG_HPP
