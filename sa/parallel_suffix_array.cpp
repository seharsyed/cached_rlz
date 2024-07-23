#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <cstdint>
#include <cstdlib>

#include "parallel_quicksort.hpp"

template<typename T>
std::vector<T> read_file(const char* filename) {
    std::ifstream ifs(filename, std::ios::binary);

    const auto begin = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    const std::size_t len = (end - begin) / sizeof(T);
    ifs.seekg(0);

    std::vector<T> v(len, 0);

    for (std::size_t i = 0; i < len; ++i) {
        ifs.read(reinterpret_cast<char*>(v.data() + i), sizeof(T));
    }

    ifs.close();

    return v;
}

template<typename T1, typename T2>
std::vector<T1> prefix_doubling(const T2* s, const std::size_t n) {
    std::vector<std::size_t> w(n, 0);

    for (std::size_t i = 0; i < n; ++i) {
        w[i] = static_cast<std::size_t>(s[i]);
    }

    std::vector<std::array<std::size_t, 3>> v(n, {0, 0, 0});

    for (std::size_t len = 1; len <= n; len <<= 1) {
        #pragma omp parallel for
        for (std::size_t i = 0; i < n; ++i) {
            if (i + len < n) {
                v[i][0] = w[i];
                v[i][1] = w[i + len];
                v[i][2] = i;
            } else {
                v[i][0] = w[i];
                v[i][1] = 0;
                v[i][2] = i;
            }
        }

        #pragma omp parallel
        #pragma omp single
        quicksort(0, v.size(), v.data());

        std::size_t name = 1;
        for (std::size_t i = 0; i < n; ++i) {
            if (i > 0 && (v[i][0] != v[i - 1][0] || v[i][1] != v[i - 1][1])) {
                ++name;
            }
            w[v[i][2]] = name;
        }
    }

    std::vector<T1> suffix_array(n, 0);

    for (T1 i = 0; i < n; ++i) {
        suffix_array[w[i] - 1] = i;
    }

    return suffix_array;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " [infile] [outfile]\n\n"
                  << "Computes the suffix array of infile and stores into outfile.\n";
        std::exit(EXIT_FAILURE);
    }

    const auto input = read_file<char>(argv[1]);
    auto sa = prefix_doubling<std::uint32_t, char>(input.data(), input.size());

    std::ofstream ofs(argv[2]);
    ofs.write(reinterpret_cast<char*>(sa.data()), sizeof(decltype(sa)::value_type) * sa.size());
    ofs.close();
}
