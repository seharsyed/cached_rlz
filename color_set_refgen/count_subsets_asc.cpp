#include <algorithm>
#include <iostream>

#include <cstdint>

#include "generate_dictionary.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s [input file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    const auto color_sets = get_color_sets<std::uint32_t>(argv[1]);
    std::size_t ssn = 0;
    std::size_t nb = 0;
    for (std::size_t i = 0; i < color_sets.size(); ++i) {
        const auto& s1 = color_sets[i];
        for (std::size_t j = i + 1; j < color_sets.size(); ++j) {
            const auto& s2 = color_sets[j];
            if (std::includes(s2.begin(), s2.end(), s1.begin(), s1.end())) {
                ++ssn;
                nb += s2.size();
                break;
            }
        }
    }

    std::cout << "number of sets: " << color_sets.size() << "\n";
    std::cout << "number of subsets: " << ssn << "\n";
    std::cout << "number of bits to encode subsets: " << nb << "\n";
}
