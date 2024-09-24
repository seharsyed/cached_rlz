#include <algorithm>
#include <bit>
#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>

#include <cstdint>

#include <sdsl/bit_vectors.hpp>
#include <sdsl/int_vector.hpp>

#include "ds.hpp"

template<typename T>
std::vector<std::vector<T>> get_color_sets(const char* input_filename) {
    std::vector<std::vector<T>> color_sets;

    std::ifstream ifs(input_filename, std::ios::binary);
    const auto begin = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    const std::size_t file_length = end - begin;
    ifs.seekg(0);
    const std::size_t text_length = file_length / sizeof(T);

    std::size_t i = 0;
    while (i < text_length) {
        std::size_t cs_sz = 0;
        ifs.read(reinterpret_cast<char*>(&cs_sz), sizeof(T));
        ++i;
        std::vector<T> cs(cs_sz, 0);
        for (std::size_t j = 0; j < cs_sz; ++j) {
            ifs.read(reinterpret_cast<char*>(&cs[j]), sizeof(T));
            ++i;
        }
        color_sets.push_back(std::move(cs));
    }
    ifs.close();

    return color_sets;
}

template<typename T>
std::vector<T> get_parents(const char* input_filename) {
    std::vector<T> parents;

    std::ifstream ifs(input_filename, std::ios::binary);
    const auto begin = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    const std::size_t file_length = end - begin;
    ifs.seekg(0);
    const std::size_t text_length = file_length / sizeof(T);

    std::size_t i = 0;
    while (i < text_length) {
        T parent = 0;
        ifs.read(reinterpret_cast<char*>(&parent), sizeof(T));
        parents.push_back(parent);
        ++i;
    }
    ifs.close();

    return parents;
}

static inline std::size_t bits_required(const std::size_t x) {
    return std::max(static_cast<std::size_t>(std::bit_width(x)), static_cast<std::size_t>(1));
}

std::vector<std::int64_t> find_parents(const std::vector<std::vector<std::uint32_t>>& color_sets) {
    // if ancestor[i] = -1, then set i is root set
    std::vector<std::int64_t> ancestor_vec(color_sets.size(), -1);

    #pragma omp parallel for schedule(dynamic, 1)
    for (std::int64_t i = 0; i < color_sets.size(); ++i) {
        const auto& s1 = color_sets[i];

        for (std::int64_t j = i + 1; j < color_sets.size(); ++j) {
            const auto& s2 = color_sets[j];

            // if |s1| >= |s2|, then s1 cannot be a subset of s2
            if (s1.size() >= s2.size()) {
                continue;
            }

            if (std::includes(s2.begin(), s2.end(), s1.begin(), s1.end())) {
                ancestor_vec[i] = j;
                break;
            }
        }
    }

    return ancestor_vec;
}

std::tuple<ds, std::vector<int64_t>> build_ds(const std::vector<std::vector<std::uint32_t>>& color_sets,
                                              std::vector<std::int64_t>& ancestor_vec,
                                              const std::int64_t enc_width) {
    std::size_t subset_count = 0;
    std::size_t subset_elements = 0;

    std::size_t dense_count = 0;
    std::size_t dense_elements = 0;

    std::size_t sparse_count = 0;
    std::size_t sparse_elements = 0;

    std::size_t root_count = 0;
    const std::size_t ptr_width = bits_required(color_sets.size());

    std::cout << "Computing space for roots\n";

    for (std::int64_t i = 0; i < ancestor_vec.size(); ++i) {
        if (ancestor_vec[i] == -1) {
            ++root_count;

            const std::size_t dense_bits = color_sets[i].back() + 1;
            const std::size_t sparse_bits = color_sets[i].size() * enc_width;

            if (dense_bits < sparse_bits) {
                ++dense_count;
                dense_elements += dense_bits;
            } else {
                ++sparse_count;
                sparse_elements += color_sets[i].size();
            }
        } else {
            const auto ancestor_idx = ancestor_vec[i];
            const std::size_t ancestor_bits = color_sets[ancestor_idx].size();
            const std::size_t ss_bits = ancestor_bits + ptr_width;

            const std::size_t dense_bits = color_sets[i].back() + 1;
            const std::size_t sparse_bits = color_sets[i].size() * enc_width;

            if ((ss_bits < dense_bits) && (ss_bits < sparse_bits)) {
                ++subset_count;
                subset_elements += ancestor_bits;
            } else {
                ancestor_vec[i] = -1;
                ++root_count;
                if (dense_bits < sparse_bits) {
                    ++dense_count;
                    dense_elements += dense_bits;
                } else {
                    ++sparse_count;
                    sparse_elements += color_sets[i].size();
                }
            }
        }
    }

    std::vector<std::int64_t> set_mapping(color_sets.size(), -1);

    std::cout << "Root sets: " << root_count << "\n";
    std::cout << "Dense root sets: " << dense_count << "\n";
    std::cout << "Sparse root sets: " << sparse_count << "\n";
    std::cout << "Subsets: " << subset_count << "\n";

    sdsl::bit_vector dense_roots(dense_elements, 0);
    sdsl::int_vector<> dense_starts(dense_count + 1, 0, bits_required(dense_elements));

    sdsl::int_vector<> sparse_roots(sparse_elements, 0, enc_width);
    sdsl::int_vector<> sparse_starts(sparse_count + 1, 0, bits_required(sparse_elements));

    sdsl::bit_vector subsets(subset_elements, 0);
    sdsl::int_vector<> subset_starts(subset_count + 1, 0, bits_required(subset_elements));
    sdsl::int_vector<> ancestor_ptrs(subset_count, 0, bits_required(color_sets.size()));

    std::cout << "Computing representation\n";

    {
        std::int64_t dense_idx = 0;
        std::int64_t sparse_idx = dense_count;
        std::int64_t subset_idx = dense_count + sparse_count;

        std::size_t dense_container_idx = 0;
        std::size_t dense_starts_idx = 1;

        std::size_t sparse_container_idx = 0;
        std::size_t sparse_starts_idx = 1;

        std::size_t subset_container_idx = 0;
        std::size_t subset_starts_idx = 1;

        for (std::int64_t i = 0; i < color_sets.size(); ++i) {
            if (ancestor_vec[i] == -1) {
                const std::int64_t sparse_bits = color_sets[i].size() * enc_width;
                const std::int64_t dense_bits = color_sets[i].back() + 1;

                if (dense_bits < sparse_bits) {
                    std::vector<bool> temp_bv(color_sets[i].back() + 1, 0);

                    for (const auto x : color_sets[i]) {
                        temp_bv[x] = 1;
                    }

                    for (const auto x : temp_bv) {
                        dense_roots[dense_container_idx++] = x;
                    }

                    dense_starts[dense_starts_idx++] = dense_container_idx;

                    set_mapping[i] = dense_idx++;
                } else {
                    for (const auto x : color_sets[i]) {
                        sparse_roots[sparse_container_idx++] = x;
                    }

                    sparse_starts[sparse_starts_idx++] = sparse_container_idx;

                    set_mapping[i] = sparse_idx++;
                }
            } else {
                const auto ancestor_idx = ancestor_vec[i];
                const std::int64_t ancestor_size = color_sets[ancestor_idx].size();
                std::vector<bool> temp_bv(ancestor_size, 0);

                for (std::int64_t m = 0, k = 0; m < ancestor_size; ++m) {
                    if (k < color_sets[i].size() && color_sets[ancestor_idx][m] == color_sets[i][k]) {
                        temp_bv[m] = 1;
                        ++k;
                    }
                }

                for (const auto x : temp_bv) {
                    subsets[subset_container_idx++] = x;
                }

                subset_starts[subset_starts_idx++] = subset_container_idx;

                set_mapping[i] = subset_idx++;
            }
        }
    }

    std::cout << "Computing ancestor pointers\n";

    {
        std::int64_t subset_idx = 0;

        for (std::int64_t i = 0; i < ancestor_vec.size(); ++i) {
            const auto ancestor = ancestor_vec[i];

            if (ancestor != -1) {
                const std::int64_t subset_id = subset_idx++;
                const std::int64_t ancestor_id = set_mapping[ancestor];
                ancestor_ptrs[subset_id] = ancestor_id;

                const auto ss_sz = subset_starts[subset_id + 1] - subset_starts[subset_id];
                const auto anc_sz = color_sets[ancestor].size();
            }
        }
    }

    return {ds(dense_roots, dense_starts, sparse_roots, sparse_starts, subsets, subset_starts, ancestor_ptrs), set_mapping};
}
