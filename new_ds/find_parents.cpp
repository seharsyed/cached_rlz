#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include <cstdint>

template<typename T>
std::vector<std::vector<T>> get_color_sets(const char* input_filename) {
    std::vector<std::vector<T>> color_sets;

    std::ifstream ifs(input_filename, std::ios::binary);
    const auto begin = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    const std::size_t file_length = end - begin;
    ifs.seekg(0);
    const std::size_t text_length = file_length / sizeof(T);

    std::size_t i = 0;
    while (i < text_length) {
        std::size_t cs_sz = 0;
        ifs.read(reinterpret_cast<char*>(&cs_sz), sizeof(T));
        ++i;
        std::vector<T> cs(cs_sz, 0);
        for (std::size_t j = 0; j < cs_sz; ++j) {
            ifs.read(reinterpret_cast<char*>(&cs[j]), sizeof(T));
            ++i;
        }
        color_sets.push_back(std::move(cs));
    }
    ifs.close();

    return color_sets;
}

std::vector<std::int64_t> find_parents(const std::vector<std::vector<std::uint32_t>>& color_sets) {
    // if ancestor[i] = -1, then set i is root set
    std::vector<std::int64_t> ancestor_vec(color_sets.size(), -1);

    #pragma omp parallel for schedule(dynamic, 1)
    for (std::int64_t i = 0; i < color_sets.size(); ++i) {
        const auto& s1 = color_sets[i];

        for (std::int64_t j = i + 1; j < color_sets.size(); ++j) {
            const auto& s2 = color_sets[j];

            // if |s1| >= |s2|, then s1 cannot be a subset of s2
            if (s1.size() >= s2.size()) {
                continue;
            }

            if (std::includes(s2.begin(), s2.end(), s1.begin(), s1.end())) {
                ancestor_vec[i] = j;
                break;
            }
        }
    }

    return ancestor_vec;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s [input file] [output file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    const auto color_sets = get_color_sets<std::uint32_t>(argv[1]);

    std::cout << "Computing parents\n";

    const std::vector<std::int64_t> parent_vec = find_parents(color_sets);

    std::cout << "Writing parents to disk\n";

    std::ofstream ofs(argv[2]);
    for (const auto parent : parent_vec) {
        ofs.write(reinterpret_cast<const char*>(&parent), sizeof(parent));
    }
    ofs.close();
}
