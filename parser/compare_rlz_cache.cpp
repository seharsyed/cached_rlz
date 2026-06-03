#include <algorithm>
#include <cstdint>
#include <optional>

#include "parser.hpp"              // baseline
#include "cached_rlz_parser.hpp"   // add-on cache wrapper

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

using Symbol = unsigned char;
using SAType = unsigned int;
using Triples = std::vector<std::tuple<std::size_t, std::size_t, std::size_t>>;

template <typename Fn>
double time_ms(Fn&& fn) {
    const auto start = std::chrono::steady_clock::now();
    fn();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

std::size_t peak_rss_kb() {
#if defined(__unix__) || defined(__APPLE__)
    struct rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
#if defined(__APPLE__)
        return static_cast<std::size_t>(usage.ru_maxrss / 1024); // macOS reports bytes
#else
        return static_cast<std::size_t>(usage.ru_maxrss);        // Linux reports KB
#endif
    }
#endif
    return 0;
}

void print_usage(const char* prog) {
    std::cerr
        << "Usage:\n"
        << "  " << prog << " <reference> <suffix-array> <input> [bucket_divisor] [trials]\n\n"
        << "For current DNA sanity run, use the same file as reference and input:\n"
        << "  " << prog << " ~/PhD/Datafiles/dna.200MB ~/PhD/Datafiles/dna.200MB.sa ~/PhD/Datafiles/dna.200MB 256 3\n";
}

int main(int argc, char** argv) {
    if (argc < 4 || argc > 6) {
        print_usage(argv[0]);
        return 1;
    }

    const char* ref_file = argv[1];
    const char* sa_file = argv[2];
    const char* input_file = argv[3];

    const std::size_t bucket_divisor =
        argc >= 5 ? static_cast<std::size_t>(std::stoull(argv[4])) : 256;

    const int trials = argc >= 6 ? std::stoi(argv[5]) : 3;

    if (bucket_divisor == 0) {
        std::cerr << "bucket_divisor must be > 0\n";
        return 1;
    }

    if (trials <= 0) {
        std::cerr << "trials must be > 0\n";
        return 1;
    }

    std::cerr << "Loading files...\n";
    auto ref = read_file<Symbol>(ref_file);
    auto sa = read_file<SAType>(sa_file);
    auto input = read_file<Symbol>(input_file);

    std::cout << "reference bytes : " << ref.size() << "\n";
    std::cout << "sa entries      : " << sa.size() << "\n";
    std::cout << "input bytes     : " << input.size() << "\n";
    std::cout << "bucket divisor  : " << bucket_divisor << "\n";
    std::cout << "trials          : " << trials << "\n\n";

    if (ref.empty() || sa.empty() || input.empty()) {
        std::cerr << "Error: reference, suffix array, and input must all be non-empty.\n";
        return 1;
    }

    if (sa.size() != ref.size()) {
        std::cerr << "Warning: SA entries (" << sa.size()
                  << ") != reference bytes (" << ref.size()
                  << "). Continue only if this is expected for your file format.\n";
    }

    Triples baseline;
    double baseline_best = 1e100;

    std::cout << "Running baseline lzFactorize...\n";
    for (int t = 1; t <= trials; ++t) {
        Triples current;

        const double ms = time_ms([&] {
            current = lzFactorize<Symbol, SAType>(input, ref, sa);
        });

        if (t == 1) {
            baseline = std::move(current);
        } else if (current != baseline) {
            std::cerr << "Error: baseline output changed between trials.\n";
            return 2;
        }

        baseline_best = std::min(baseline_best, ms);

        std::cout << "  baseline trial " << t << ": "
                  << std::fixed << std::setprecision(3)
                  << ms << " ms\n";
    }

    CachedRLZParser<Symbol, SAType> parser(ref, sa, bucket_divisor);
    Triples cached;
    double cached_best = 1e100;

    std::cout << "\nRunning cached lzFactorize...\n";
    CachedRLZParser<Symbol, SAType>::CacheInfo final_info{};

    for (int t = 1; t <= trials; ++t) {
        parser.clear_cache();
        Triples current;

        const double ms = time_ms([&] {
            current = parser.lzFactorize(input, ref, sa);
        });

        if (t == 1) {
            cached = std::move(current);
        } else if (current != cached) {
            std::cerr << "Error: cached output changed between trials.\n";
            return 3;
        }

        cached_best = std::min(cached_best, ms);
        final_info = parser.cache_info();

        std::cout << "  cached trial " << t << ":   "
                  << std::fixed << std::setprecision(3)
                  << ms << " ms"
                  << "  hits=" << final_info.hits
                  << " misses=" << final_info.misses
                  << " entries=" << final_info.current_size
                  << "\n";
    }

    const bool equal = (baseline == cached);

    std::cout << "baseline factors   : " << baseline.size() << "\n";
    std::cout << "cached factors     : " << cached.size() << "\n";

    std::cout << "\n=== RESULT ===\n";
    std::cout << "outputs equal       : " << (equal ? "YES" : "NO") << "\n";
    std::cout << "phrases             : " << baseline.size() << "\n";
    std::cout << "baseline best ms    : "
              << std::fixed << std::setprecision(3)
              << baseline_best << "\n";
    std::cout << "cached best ms      : "
              << std::fixed << std::setprecision(3)
              << cached_best << "\n";
    std::cout << "speedup             : "
              << std::fixed << std::setprecision(3)
              << (cached_best > 0.0 ? baseline_best / cached_best : 0.0)
              << "x\n";
    std::cout << "cache hits          : " << final_info.hits << "\n";
    std::cout << "cache misses        : " << final_info.misses << "\n";
    std::cout << "cache hit rate      : "
              << std::fixed << std::setprecision(6)
              << final_info.hit_rate << "\n";
    std::cout << "cache entries       : " << final_info.current_size << "\n";
    std::cout << "cache buckets       : " << final_info.bucket_count << "\n";
    std::cout << "max bucket size     : " << final_info.max_bucket_size << "\n";
    std::cout << "cache load factor   : "
              << std::fixed << std::setprecision(3)
              << final_info.load_factor << "\n";
    std::cout << "approx cache MB     : "
              << std::fixed << std::setprecision(3)
              << final_info.approx_bytes / (1024.0 * 1024.0) << "\n";
    std::cout << "peak RSS MB         : "
              << std::fixed << std::setprecision(2)
              << peak_rss_kb() / 1024.0 << "\n";

    return equal ? 0 : 4;
}