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
    std::vector<bool> root_vec(color_sets.size(), true);
    std::vector<std::int32_t> tree_depths(color_sets.size(), 1);

    std::size_t ssn = 0;
    std::size_t nb = 0;

    std::size_t sparse_sets = 0;
    std::size_t sparse_elements = 0;
    constexpr std::size_t enc_width = 16;

    std::size_t dense_sets = 0;
    std::size_t dense_elements = 0;

    #pragma omp parallel for schedule(dynamic, 1) shared(color_sets, root_vec, tree_depths, ssn, nb)
    for (std::size_t i = 0; i < color_sets.size(); ++i) {
        const auto& s1 = color_sets[i];

        for (std::size_t j = i + 1; j < color_sets.size(); ++j) {
            const auto& s2 = color_sets[j];

            if (s1.size() >= s2.size()) {
                continue;
            }

            if (std::includes(s2.begin(), s2.end(), s1.begin(), s1.end())) {
                #pragma omp critical
                {

                    const std::size_t sparse_sz = s1.size();
                    const std::size_t dense_sz = s2.size();
                    const std::size_t sparse_bits = sparse_sz * enc_width;
                    const std::size_t dense_bits = dense_sz;

                    if (sparse_bits <= dense_bits) {
                        ++ssn;
                        ++sparse_sets;
                        sparse_elements += sparse_sz;
                        nb += sparse_bits;
                    } else {
                        ++ssn;
                        ++dense_sets;
                        dense_elements += dense_sz;
                        nb += dense_bits;
                        root_vec[i] = false;
                        if (tree_depths[j] <= tree_depths[i]) {
                            tree_depths[j] = tree_depths[i] + 1;
                        }
                    }
                }
                break;
            }
        }
    }

    const auto max_td = *std::max_element(tree_depths.begin(), tree_depths.end());

    std::size_t root_sets = 0;
    std::size_t rse = 0;
    std::int64_t dcs = 0;
    std::int64_t nt = 0;

    for (std::size_t i = 0; i < tree_depths.size(); ++i) {
        if (root_vec[i]) {
            rse += color_sets[i].size();
            dcs += tree_depths[i];
            ++nt;
        }
    }

    const auto mean_td = static_cast<double>(dcs) / static_cast<double>(nt);

    std::cout << "number of sets: " << color_sets.size() << "\n";
    std::cout << "number of subsets: " << ssn << "\n";
    std::cout << "number of bits to encode subsets: " << nb << "\n";
    std::cout << "total size of root sets: " << rse << "\n";

    std::cout << "number of trees: " << nt << "\n";
    std::cout << "maximum tree depth: " << max_td << "\n";
    std::cout << "mean tree depth: " << mean_td << "\n";

    std::cout << "sparse sets: " << sparse_sets << "\n";
    std::cout << "sparse elements: " << sparse_elements << "\n";

    std::cout << "dense sets: " << dense_sets << "\n";
    std::cout << "dense elements: " << dense_elements << "\n";
}
