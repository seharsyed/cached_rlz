#include <fstream>
#include <iostream>

#include "ds.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s [input file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    std::ifstream ifs(argv[1]);

    ds d;
    d.load(ifs);
    ifs.close();

    const auto m = d.space_breakdown();
    for (const auto& [name, space]: m) {
        std::cout << name << ": " << space << "\n";
    }

    std::cout << "dense: " << d.dense_starts.size() << ", " << d.dense_starts[d.dense_starts.size() - 1] << "\n";
    std::cout << "sparse: " << d.sparse_starts.size() << ", " << d.sparse_starts[d.sparse_starts.size() - 1] << "\n";
    std::cout << "subset: " << d.subset_starts.size() << ", " << d.subset_starts[d.subset_starts.size() - 1] << "\n";

    std::size_t colors = 0;
    for (std::size_t i = 0; i < d.dense_container.size(); ++i) {
        colors += d.dense_container[i];
    }

    colors += d.sparse_container.size();

    for (std::size_t i = 0; i < d.subset_container.size(); ++i) {
        colors += d.subset_container[i];
    }

    std::cout << "number of colors in sets: " << colors << "\n";

    const double bits = static_cast<double>(d.size_in_bytes()) * 8.0;

    const double bpc = bits / static_cast<double>(colors);
    std::cout << "bits per color: " << bpc << "\n";
}
