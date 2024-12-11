#include <fstream>
#include <iostream>

#include "ds_construction.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s [input file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    const auto parents = get_parents<std::int64_t>(argv[1]);
    const std::size_t sets = parents.size();
    std::size_t roots = 0;
    for (const auto p : parents) {
        if (p == -1) {
            ++roots;
        }
    }

    const std::size_t subsets = sets - roots;

    std::cout << "number of sets: " << sets << "\n";
    std::cout << "number of roots: " << root_sets << "\n";
    std::cout << "number of subsets: " << subsets << "\n";
}
