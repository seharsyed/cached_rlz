#include <algorithm>
#include <iostream>

#include <cstdint>

#include "generate_dictionary.hpp"

bool contains(const auto& s1, const auto& s2) {
    return std::search(s1.begin(), s1.end(), s2.begin(), s2.end()) != s1.end();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s [input file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    const auto color_sets = get_color_sets<std::uint32_t>(argv[1]);
    std::vector<bool> skip_vec(color_sets.size(), false);

    std::size_t ssn = 0;
    for (std::size_t i = 0; i < color_sets.size(); ++i) {
        if (skip_vec[i]) {
            continue;
        }

        const auto& s1 = color_sets[i];
        for (std::size_t j = i + 1; j < color_sets.size(); ++j) {
            if (skip_vec[j]) {
                continue;
            }

            const auto& s2 = color_sets[j];
            if (contains(s1, s2)) {
                ++ssn;
                skip_vec[j] = true;
            }
        }

        skip_vec[i] = true;
    }

    std::cout << "number of strings: " << color_sets.size() << "\n";
    std::cout << "number of substrings: " << ssn << "\n";
}
