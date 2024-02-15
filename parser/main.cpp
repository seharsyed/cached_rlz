#include <iostream>
#include <span>

#include "parser.hpp"
#include "random_access_rlz.hpp"

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 5) {
        std::fprintf(stderr, "usage: %s [reference file] [suffix array file] [input file] {output file}\n", argv[0]);
        std::exit(1);
    }

    auto ref_vec = read_file<unsigned char>(argv[1]);
    auto sa_vec = read_file<unsigned int>(argv[2]);
    auto input_vec = read_file<unsigned char>(argv[3]);
    auto res = lzFactorize<unsigned char, unsigned int>(input_vec.data(), input_vec.size(), ref_vec.data(), ref_vec.size(), sa_vec.data());

    random_access_rlz<unsigned char> rrlz(ref_vec, res);

    for (std::size_t i = 0; i < input_vec.size(); ++i) {
        if (rrlz.access(i) != input_vec[i]) {
            std::cout << "i: " << i << " | " << rrlz.access(i) << " != " << input_vec[i] << "\n";
        }
    }

    if (argc == 5) {
        std::ofstream ofs(argv[4]);
        for (const auto& [start, pos, len] : res) {
            ofs << "(" << start << ", " << pos << ", " << len << ")\n";
        }
        ofs.close();
    }
}
