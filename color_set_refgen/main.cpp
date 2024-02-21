#include <iostream>

#include <cstdint>

#include "generate_dictionary.hpp"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::fprintf(stderr, "usage: %s [input file] [number of samples] [output file] [seed]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    const char* input_filename = argv[1];
    std::size_t n_samples = static_cast<size_t>(std::atoi(argv[2]));
    const char* output_filename = argv[3];
    auto seed = static_cast<std::size_t>(std::atoi(argv[4]));

    const auto color_set_indices = get_color_set_indices<std::uint32_t>(input_filename);
    if (n_samples > color_set_indices.size()) {
        std::cerr << "Number of samples is grater than number of possible sampling positions!\n";
        std::cerr << "Number of possible sampling positions: " << color_set_indices.size() << "\n";
        std::exit(EXIT_FAILURE);
    }

    const auto sampling_positions = generate_sampling_positions(n_samples, color_set_indices.size(), seed);
    generate_dictionary<std::uint32_t>(input_filename, color_set_indices, sampling_positions, output_filename);
}
