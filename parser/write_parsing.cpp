#include <iostream>
#include <vector>

#include <cstdint>

#include "color_set_parser.hpp"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::fprintf(stderr, "usage: %s [reference file] [suffix array file] [input file] [output file]\n", argv[0]);
        std::exit(1);
    }

    auto ref_vec = read_file<std::uint32_t>(argv[1]);
    auto sa_vec = read_file<std::uint32_t>(argv[2]);
    const char* input_filename = argv[3];

    const auto res = lzFactorizeColors(input_filename, ref_vec.data(), ref_vec.size(), sa_vec.data());

    std::ofstream ofs(argv[4]);
    for (auto [start, pos, len] : res) {
        ofs.write(reinterpret_cast<char*>(&start), sizeof(std::size_t));
        ofs.write(reinterpret_cast<char*>(&pos), sizeof(std::size_t));
        ofs.write(reinterpret_cast<char*>(&len), sizeof(std::size_t));
    }
    ofs.close();
}
