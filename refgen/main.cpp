#include <cstdint>
#include <cstdlib>

#include "generate_dictionary.hpp"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::fprintf(stderr, "usage: %s [input file] [number of samples] [sample length] [output file] [seed]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    const char* input_filename = argv[1];
    std::size_t n_samples = static_cast<size_t>(std::atoi(argv[2]));
    std::size_t sample_size = static_cast<size_t>(std::atoi(argv[3]));
    const char* output_filename = argv[4];
    auto seed = static_cast<unsigned int>(std::atoi(argv[5]));
    srand(seed);

    generate_dictionary<std::uint32_t>(input_filename, n_samples, sample_size, output_filename);
}
