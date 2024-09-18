#include <algorithm>
#include <bit>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <cassert>
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

static inline std::size_t bits_required(const std::size_t x) {
    return std::max(static_cast<std::size_t>(std::bit_width(x)), static_cast<std::size_t>(1));
}

std::tuple<ds, std::vector<int64_t>> build_ds(const std::vector<std::vector<std::uint32_t>>& color_sets,
                                              const std::size_t depth_limit,
                                              const std::size_t enc_width) {
    // if ancestor[i] = -1, then set i is root set
    std::vector<std::int64_t> ancestor_vec(color_sets.size(), -1);
    std::vector<std::size_t> depth_vec(color_sets.size(), 1);

    std::cout << "Computing ancestors\n";

    const std::size_t ptr_width = bits_required(color_sets.size());

    if (depth_limit > 1) {
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
                    #pragma omp critical
                    {
                        const std::size_t sparse_bits = s1.size() * enc_width;
                        const std::size_t dense_bits = s2.size();

                        if (((dense_bits + ptr_width) < sparse_bits) && (depth_vec[i] + 1 <= depth_limit)) {
                            ancestor_vec[i] = j;
                            if (depth_vec[j] <= depth_vec[i]) {
                                const auto d = depth_vec[i] + 1;
                                depth_vec[j] = d;
                            }
                        }

                    }

                    if (ancestor_vec[i] != -1) {
                        break;
                    }
                }
            }
        }
    }

    std::size_t subset_count = 0;
    std::size_t subset_elements = 0;

    std::size_t dense_count = 0;
    std::size_t dense_elements = 0;

    std::size_t sparse_count = 0;
    std::size_t sparse_elements = 0;

    std::size_t root_count = 0;

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
            const std::size_t dense_bits = color_sets[ancestor_idx].size();

            ++subset_count;
            subset_elements += dense_bits;
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

                assert((ss_sz == anc_sz) && "Subset length was not size of it's ancestor");
            }
        }
    }

    return {ds(dense_roots, dense_starts, sparse_roots, sparse_starts, subsets, subset_starts, ancestor_ptrs), set_mapping};
}

template<typename T>
void print_vec(const std::vector<T>& v) {
    for (const auto x : v) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}

template<typename T>
std::string stringify_vec(const std::vector<T>& v) {
    std::stringstream ss;
    ss << "{";
    for (std::size_t i = 0; i < v.size() - 1; ++i) {
        ss << v[i] << ", ";
    }
    ss << v.back() << "}";

    return ss.str();
}

void test_run() {
    std::vector<std::vector<std::uint32_t>> color_sets;

    color_sets.push_back({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    color_sets.push_back({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11});
    color_sets.push_back({0, 1, 2, 3, 4, 5});
    color_sets.push_back({0, 1, 2});
    color_sets.push_back({0, 1});
    color_sets.push_back({0, 2, 4, 6, 8, 10, 12, 14});
    color_sets.push_back({0, 4, 8, 12});
    color_sets.push_back({0, 8});
    color_sets.push_back({0});
    color_sets.push_back({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    color_sets.push_back({1, 2});
    color_sets.push_back({1, 3, 5, 7, 9, 11, 13, 15});
    color_sets.push_back({1, 5, 9, 13});
    color_sets.push_back({1, 9});
    color_sets.push_back({10, 11});
    color_sets.push_back({10});
    color_sets.push_back({11});
    color_sets.push_back({12});
    color_sets.push_back({13});
    color_sets.push_back({14});
    color_sets.push_back({15});
    color_sets.push_back({1});
    color_sets.push_back({2, 10});
    color_sets.push_back({2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15});
    color_sets.push_back({2, 6, 10, 15});
    color_sets.push_back({2, 8, 9, 10, 11, 12, 13, 15});
    color_sets.push_back({3, 11});
    color_sets.push_back({3, 4, 5});
    color_sets.push_back({3, 4});
    color_sets.push_back({3, 7, 11, 15});
    color_sets.push_back({3});
    color_sets.push_back({4, 12});
    color_sets.push_back({4, 5});
    color_sets.push_back({4});
    color_sets.push_back({5, 13});
    color_sets.push_back({6, 15});
    color_sets.push_back({7, 15});

    std::sort(color_sets.begin(), color_sets.end(),
              [](const std::vector<std::uint32_t>& v, const std::vector<std::uint32_t>& w) {
                  return v.size() < w.size();
              });

    const std::int32_t depth_limit = 3;
    const std::int64_t enc_width = 4;

    const auto [d, m] = build_ds(color_sets, depth_limit, enc_width);

    std::cout << "d.dense_container.size() "  << d.dense_container.size()  << "\n";
    std::cout << "d.dense_starts.size() "     << d.dense_starts.size()     << "\n";
    std::cout << "d.sparse_container.size() " << d.sparse_container.size() << "\n";
    std::cout << "d.sparse_starts.size() "    << d.sparse_starts.size()    << "\n";
    std::cout << "d.subset_container.size() " << d.subset_container.size() << "\n";
    std::cout << "d.subset_starts.size() "    << d.subset_starts.size()    << "\n";
    std::cout << "d.ancestor_ptrs.size() "    << d.ancestor_ptrs.size()    << "\n";
    std::cout << "\n";

    for (std::int64_t i = 0; i < m.size(); ++i) {
        std::cout << i << " -> " << m[i] << "\n";
        const auto idx = m[i];
        const auto s = d.extract(idx);
        if (color_sets[i] != s) {
            std::cout << stringify_vec(color_sets[i]) << " != " << stringify_vec(s) << "\n";
        }
    }

    std::cout << "size in bytes: " << d.size_in_bytes() << "\n";

    for (auto i = 0; i < d.ancestor_ptrs.size(); ++i) {
        std::cout << d.ancestor_ptrs[i] << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::fprintf(stderr, "usage: %s [input file] [depth limit] [output file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    const auto color_sets = get_color_sets<std::uint32_t>(argv[1]);
    const std::int32_t depth_limit = std::stoi(argv[2]);

    std::int64_t enc_width = 0;

    for (const auto& cs : color_sets) {
        for (const auto x : cs) {
            const std::int64_t bits = bits_required(x);
            enc_width = std::max(enc_width, bits);
        }
    }

    std::cout << "depth limit: " << depth_limit << "\n";
    std::cout << "encoding width: " << enc_width << "\n";

    const auto [d, m] = build_ds(color_sets, depth_limit, enc_width);

    std::cout << "d.dense_container.size() "  << d.dense_container.size()  << "\n";
    std::cout << "d.dense_starts.size() "     << d.dense_starts.size()     << "\n";
    std::cout << "d.sparse_container.size() " << d.sparse_container.size() << "\n";
    std::cout << "d.sparse_starts.size() "    << d.sparse_starts.size()    << "\n";
    std::cout << "d.subset_container.size() " << d.subset_container.size() << "\n";
    std::cout << "d.subset_starts.size() "    << d.subset_starts.size()    << "\n";
    std::cout << "d.ancestor_ptrs.size() "    << d.ancestor_ptrs.size()    << "\n";
    std::cout << "\n";
    std::cout << "size in bytes: " << d.size_in_bytes() << "\n";

    std::ofstream ofs(argv[3]);
    const auto bw = d.serialize(ofs);
    std::cout << "bytes written: " << bw << "\n";
    ofs.close();
}
