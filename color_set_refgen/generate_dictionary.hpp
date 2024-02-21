#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <vector>

template<typename T>
std::vector<std::size_t> get_color_set_indices(const char* input_filename) {
    std::vector<std::size_t> cs_indices;

    std::ifstream ifs(input_filename, std::ios::binary);
    const auto begin = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    const std::size_t file_length = end - begin;
    ifs.seekg(0);
    const std::size_t text_length = file_length / sizeof(T);

    std::size_t i = 0;
    while (i < text_length) {
        cs_indices.push_back(i);
        std::size_t cs_sz = 0;
        ifs.read(reinterpret_cast<char*>(&cs_sz), sizeof(T));
        ++i;
        for (std::size_t j = 0; j < cs_sz; ++j) {
            ifs.read(reinterpret_cast<char*>(&cs_sz), sizeof(T));
            ++i;
        }
    }
    ifs.close();

    return cs_indices;
}

std::vector<std::size_t> generate_sampling_positions(const std::size_t n, const std::size_t sz, const std::size_t seed) {
    std::random_device rd;
    std::mt19937 gen(rd());
    gen.seed(seed);
    std::uniform_int_distribution<std::size_t> distribution(0, sz - 1);

    std::set<std::size_t> sampling_positions;
    while (sampling_positions.size() < n) {
        sampling_positions.insert(distribution(gen));
    }

    std::vector<std::size_t> v;
    v.reserve(n);

    for (const auto e : sampling_positions) {
        v.push_back(e);
    }

    return v;
}

template<typename T>
void generate_dictionary(const char* input_filename,
                         const std::vector<std::size_t>& color_set_indices,
                         const std::vector<std::size_t>& sampling_positions,
                         const char* output_filename) {
    std::ifstream ifs(input_filename, std::ios::binary);
    std::ofstream ofs(output_filename, std::ios::binary);
    for (std::size_t i = 0; i < sampling_positions.size(); ++i) {
        ifs.seekg(color_set_indices[sampling_positions[i]] * sizeof(T));
        T color_set_sz = 0;
        ifs.read(reinterpret_cast<char*>(&color_set_sz), sizeof(T));
        std::vector<T> color_set(color_set_sz, 0);
        ifs.read(reinterpret_cast<char*>(color_set.data()), sizeof(T) * color_set_sz);
        ofs.write(reinterpret_cast<char*>(color_set.data()), sizeof(T) * color_set_sz);
    }
    ifs.close();
    ofs.close();
}
