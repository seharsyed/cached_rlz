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
    std::vector<std::int64_t> root_vec(color_sets.size(), -1);

    // root set elems
    std::size_t rse = 0;
    // subset count
    std::size_t ssn = 0;
    // subset bits
    std::size_t nb = 0;

    #pragma omp parallel for schedule(dynamic, 1) shared(ssn, root_vec)
    for (std::size_t i = 0; i < color_sets.size(); ++i) {
        if (root_vec[i] != -1) {
            continue;
        }

        const auto& s1 = color_sets[i];
        for (std::size_t j = i + 1; j < color_sets.size(); ++j) {
            if (root_vec[j] != -1) {
                continue;
            }

            const auto& s2 = color_sets[j];
            if (std::includes(s1.begin(), s1.end(), s2.begin(), s2.end())) {
                #pragma omp critical
                {
                    ++ssn;
                    root_vec[j] = i;
                }
            }
        }
    }

    std::vector<std::int64_t> ancestor_vec(color_sets.size(), -1);
    std::vector<bool> leaf_vec(color_sets.size(), true);

    #pragma omp parallel for schedule(dynamic, 1) shared(rse, nb, root_vec, ancestor_vec, leaf_vec)
    for (std::int64_t i = static_cast<std::int64_t>(color_sets.size()); i >= 0; --i) {
        const auto& s1 = color_sets[i];

        if (root_vec[i] == -1) {
            #pragma omp critical
            {
                rse += s1.size();
                leaf_vec[i] = false;
            }
            continue;
        }

        for (std::int64_t j = i - 1; j >= 0; --j) {
            if (root_vec[i] != root_vec[j]) {
                continue;
            }

            const auto& s2 = color_sets[j];
            if (std::includes(s2.begin(), s2.end(), s1.begin(), s1.end())) {
                #pragma omp critical
                {
                    nb += s2.size();
                    ancestor_vec[i] = j;
                    leaf_vec[j] = false;
                }
                break;
            }
        }
    }

    std::vector<std::int64_t> tree_depths(color_sets.size(), 0);

    #pragma omp parallel for schedule(dynamic, 1) shared(root_vec, ancestor_vec, leaf_vec, tree_depths)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(color_sets.size()); ++i) {
        if (!leaf_vec[i]) {
            continue;
        }

        std::int64_t tp = i;
        std::int64_t d = 0;
        while (root_vec[tp] != -1) {
            tp = ancestor_vec[i];
            ++d;
        }

        #pragma omp critical
        {
            tree_depths[tp] = std::max(d, tree_depths[tp]);
        }
    }

    const auto max_td = *std::max_element(tree_depths.begin(), tree_depths.end());

    std::int64_t dcs = 0;
    std::int64_t nt = 0;
    for (const auto d : tree_depths) {
        if (d > 0) {
            dcs += d;
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
}
