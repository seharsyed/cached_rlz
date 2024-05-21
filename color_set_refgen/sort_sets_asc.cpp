#include <algorithm>
#include <iostream>

#include <cstdint>

#include "generate_dictionary.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s [input file] [output file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    write_sets_asc<std::uint32_t>(argv[1], argv[2]);
}
