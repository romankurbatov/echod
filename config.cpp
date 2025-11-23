#include "config.hpp"

#include <iostream>
#include <unistd.h>
#include <getopt.h>

Config::Config(int argc, char *argv[]) :
        m_need_help(false)
{
    m_valid = parse(argc, argv);
}

bool Config::parse(int argc, char *argv[]) {
    option long_options[]{
        { "help", no_argument, nullptr, 'h' },
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_options, nullptr)) != -1) {
        switch (opt) {
        case 'h':
            m_need_help = true;
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

void Config::show_help() {
    std::cerr << "echod -- echo server with several commands" << std::endl;
    std::cerr << "-h|--help -- print this message" << std::endl;
}
