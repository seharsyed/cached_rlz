#include <iostream>

#include <cstdint>

#include "color_set_parser.hpp"
#include "random_access_rlz.hpp"

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 5) {
        std::fprintf(stderr, "usage: %s [reference file] [suffix array file] [input file] {output file}\n", argv[0]);
        std::exit(1);
    }

    auto ref_vec = read_file<std::uint32_t>(argv[1]);
    auto sa_vec = read_file<std::uint32_t>(argv[2]);
    const char* input_filename = argv[3];

    const auto res = lzFactorizeColors(input_filename, ref_vec.data(), ref_vec.size(), sa_vec.data());

    random_access_rlz<std::uint32_t> rrlz(ref_vec, res);

    const auto [last_start, last_pos, last_len] = res.back();
    const std::size_t decompressed_sz = last_start + last_len;
    const auto avrg_phrase_len = static_cast<double>(decompressed_sz) / static_cast<double>(res.size());

    std::size_t mismatches = 0;
    for (const auto [start, pos, len] : res) {
        if (len == 1) {
            ++mismatches;
        }
    }

    std::cout << "size of index: " << rrlz.size_in_bytes() << "\n";
    std::cout << "number of phrases: " << res.size() << "\n";
    std::cout << "average phrase length: " << avrg_phrase_len << "\n";
    std::cout << "length 1 matches: " << mismatches << "\n";
}
