#ifndef CONFIG_HPP
#define CONFIG_HPP

class Config {
public:
    Config(int argc, char *argv[]);

    Config(const Config &) = delete;
    void operator=(const Config &) = delete;

    bool is_valid() const { return m_valid; }
    bool need_help() const { return m_need_help; }

    static void show_help();

private:
    bool parse(int argc, char *argv[]);

    bool m_need_help;
    bool m_valid;
};

#endif // CONFIG_HPP
