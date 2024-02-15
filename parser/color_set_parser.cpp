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

    std::cout << rrlz.size_in_bytes() << "\n";
}
