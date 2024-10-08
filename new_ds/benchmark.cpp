#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <cstdint>

#include "ds.hpp"

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

double extract_benchmark(const ds& d, const std::size_t input_sz, const std::size_t n) {
    const auto sampling_positions = generate_sampling_positions(n, input_sz);

    auto start = std::chrono::high_resolution_clock::now();
    std::uint32_t x = 0;
    for (const auto pos : sampling_positions) {
        const auto res{d.extract(pos)};
        x ^= res.back(); // in order to not optimize result away
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "x: " << x << "\n";

    return duration.count();
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s [input file] [number of accesses]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    std::ifstream ifs(argv[1]);

    ds d;
    d.load(ifs);
    ifs.close();

    const std::size_t accesses = std::stoull(argv[2]);

    const auto duration = extract_benchmark(d, d.size(), accesses);
    std::cout << accesses << " accesses took: " <<  duration << " seconds\n";
    std::cout << "average time per access: " <<  duration / static_cast<double>(accesses) << " seconds\n";
}
