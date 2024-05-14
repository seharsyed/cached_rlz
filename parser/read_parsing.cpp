#include <iostream>
#include <vector>

#include <cstdint>

#include <sdsl/int_vector.hpp>

#include "color_set_parser.hpp"
#include "random_access_rlz.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s [reference file] [parsing file]\n", argv[0]);
        std::exit(1);
    }

    auto ref_vec = read_file<std::uint32_t>(argv[1]);
    auto parsing_vec = read_file<std::size_t>(argv[2]);
    std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> spl_vec;
    for (std::size_t i = 0; i < parsing_vec.size() / 3; ++i) {
        const auto start = parsing_vec[i + 0];
        const auto pos = parsing_vec[i + 1];
        const auto len = parsing_vec[i + 2];
        spl_vec.push_back({start, pos, len});
    }

    random_access_rlz<std::uint32_t> rrlz(ref_vec, spl_vec);

    const auto [last_start, last_pos, last_len] = spl_vec.back();
    const std::size_t decompressed_sz = last_start + last_len;
    const auto avrg_phrase_len = static_cast<double>(decompressed_sz) / static_cast<double>(spl_vec.size());
    std::size_t mismatches = 0;
    sdsl::bit_vector cov_bv(ref_vec.size());
    for (const auto [start, pos, len] : spl_vec) {
        if (len == 1) {
            ++mismatches;
        } else {
            for (std::size_t i = 0; i < len; ++i) {
                cov_bv[pos + i] = 1;
            }
        }
    }

    std::size_t ones = 0;
    for (const auto& b : cov_bv) {
        ones += b;
    }

    const double coverage = static_cast<double>(ones) / static_cast<double>(ref_vec.size());

    std::cout << "size of index: " << rrlz.size_in_bytes() << " bytes\n";
    std::cout << "size of reference: " << rrlz.ref_vec.size() * sizeof(std::uint32_t) << " bytes\n";
    std::cout << "size of reference pointers: " << rrlz.ref_ptrs.size() * sizeof(std::size_t) << " bytes\n";
    std::cout << "size of starts: " << rrlz.starts.size() / 8 << " bytes\n";
    std::cout << "number of phrases: " << spl_vec.size() << "\n";
    std::cout << "average phrase length: " << avrg_phrase_len << "\n";
    std::cout << "length 1 matches: " << mismatches << "\n";
    std::cout << "reference covered: " << coverage << "\n";
}
