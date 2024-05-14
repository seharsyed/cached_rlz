#include <iostream>
#include <span>

#include <cstdint>

#include <sdsl/int_vector.hpp>

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

    const auto [last_start, last_pos, last_len] = res.back();
    const std::size_t decompressed_sz = last_start + last_len;
    const auto avrg_phrase_len = static_cast<double>(decompressed_sz) / static_cast<double>(res.size());

    std::size_t mismatches = 0;
    sdsl::bit_vector cov_bv(ref_vec.size());
    for (const auto [start, pos, len] : res) {
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
    std::cout << "number of phrases: " << res.size() << "\n";
    std::cout << "average phrase length: " << avrg_phrase_len << "\n";
    std::cout << "length 1 matches: " << mismatches << "\n";
    std::cout << "reference covered: " << coverage << "\n";

    // for (std::size_t i = 0; i < input_vec.size(); ++i) {
    //     if (rrlz.access(i) != input_vec[i]) {
    //         std::cout << "i: " << i << " | " << rrlz.access(i) << " != " << input_vec[i] << "\n";
    //     }
    // }

    if (argc == 5) {
        std::ofstream ofs(argv[4]);
        for (const auto& [start, pos, len] : res) {
            ofs << "(" << start << ", " << pos << ", " << len << ")\n";
        }
        ofs.close();
    }
}
