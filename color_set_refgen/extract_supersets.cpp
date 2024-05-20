#include <algorithm>
#include <fstream>
#include <iostream>

#include <cstdint>

#include "generate_dictionary.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s [input file] [output file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    auto color_sets = get_color_sets<std::uint32_t>(argv[1]);
    std::vector<bool> skip_vec(color_sets.size(), false);

    std::size_t ssn = 0;
    std::size_t nb = 0;
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
            if (std::includes(s1.begin(), s1.end(), s2.begin(), s2.end())) {
                ++ssn;
                nb += s1.size();
                skip_vec[j] = true;
            }
        }
    }

    std::cout << "number of sets: " << color_sets.size() << "\n";
    std::cout << "number of subsets: " << ssn << "\n";
    std::cout << "number of bits to encode subsets: " << nb << "\n";

    std::ofstream ofs(argv[2], std::ios::binary);
    for (std::size_t i = 0; i < color_sets.size(); ++i) {
        if (skip_vec[i]) {
            continue;
        }

        auto& color_set = color_sets[i];
        const std::size_t color_set_sz = color_set.size();

        ofs.write(reinterpret_cast<char*>(color_set.data()), sizeof(std::uint32_t) * color_set_sz);
    }
    ofs.close();

}
