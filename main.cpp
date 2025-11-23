#include <iostream>
#include <stdexcept>
#include <stdlib.h>

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

    return EXIT_SUCCESS;
}
