#include <chrono>
#include <iostream>
#include <random>
#include <span>

#include "parser.hpp"
#include "random_access_rlz.hpp"

std::vector<std::size_t> generate_sampling_positions(const std::size_t n, const std::size_t sz) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> distribution(0, sz - 1);

    std::vector<std::size_t> sampling_positions;
    sampling_positions.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        sampling_positions.push_back(distribution(gen));
    }

    return sampling_positions;
}

template<typename T>
double access_1_benchmark(const random_access_rlz<T>& rrlz, const std::size_t input_sz) {
    const auto sampling_positions = generate_sampling_positions(10'000, input_sz);

    auto start = std::chrono::high_resolution_clock::now();
    std::size_t x = 0;
    for (const auto pos : sampling_positions) {
        x += static_cast<std::size_t>(rrlz.access(pos));
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << x << "\n";

    return duration.count();
}

template<typename T>
double access_10_benchmark(const random_access_rlz<T>& rrlz, const std::size_t input_sz) {
    const auto sampling_positions = generate_sampling_positions(10'000, input_sz - 9);
    std::vector<std::span<const T>> buf;
    buf.reserve(10'000);

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto pos : sampling_positions) {
        rrlz.get_spans(pos, 10, buf);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << buf.back().back() << "\n";

    return duration.count();
}

template<typename T>
double access_100_benchmark(const random_access_rlz<T>& rrlz, const std::size_t input_sz) {
    const auto sampling_positions = generate_sampling_positions(10'000, input_sz - 99);
    std::vector<std::span<const T>> buf;
    buf.reserve(10'000);

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto pos : sampling_positions) {
        rrlz.get_spans(pos, 100, buf);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << buf.back().back() << "\n";

    return duration.count();
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::fprintf(stderr, "usage: %s [reference file] [suffix array file] [input file]\n", argv[0]);
        std::exit(1);
    }

    auto ref_vec = read_file<unsigned char>(argv[1]);
    auto sa_vec = read_file<unsigned int>(argv[2]);
    auto input_vec = read_file<unsigned char>(argv[3]);
    auto res = lzFactorize(input_vec, ref_vec, sa_vec);

    random_access_rlz<unsigned char> rrlz(ref_vec, res);
    std::cout << "Access length 1: " << access_1_benchmark(rrlz, input_vec.size()) << " milliseconds\n";
    std::cout << "Access length 10: " << access_10_benchmark(rrlz, input_vec.size()) << " milliseconds\n";
    std::cout << "Access length 100: " << access_100_benchmark(rrlz, input_vec.size()) << " milliseconds\n";
    std::cout << "Size of index in bytes: " << rrlz.size_in_bytes() << "\n";
    std::cout << "Size of parsing: " << res.size() << "\n";
}
