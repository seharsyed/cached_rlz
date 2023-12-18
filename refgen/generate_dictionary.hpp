#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include <cstdint>
#include <cstdlib>

std::vector<std::size_t> generate_sampling_position_set(const std::size_t sample_count,
                                                        const std::size_t sample_size,
                                                        const std::size_t text_length) {
    std::vector<std::size_t> sampling_pos_set(sample_count, 0);

    for (std::size_t i = 0; i < sample_count; ++i) {
        sampling_pos_set[i] = rand() % (text_length - sample_size);
    }

    std::sort(sampling_pos_set.begin(), sampling_pos_set.end());

    return sampling_pos_set;
}

template<typename T>
void fill_dictionary(std::vector<T>& dictionary,
                     std::ifstream& ifs,
                     const std::vector<std::size_t>& sampling_pos_set,
                     const std::size_t sample_size) {
    for (std::size_t i = 0; i < sampling_pos_set.size(); ++i) {
        ifs.seekg(sampling_pos_set[i] * sizeof(T));
        ifs.read(reinterpret_cast<char*>(dictionary.data() + i * sample_size), sizeof(T) * sample_size);
    }
}

template<typename T>
void output_dictionary(const char* output_filename,
                       std::vector<T>& dictionary,
                       const std::size_t dict_size) {
    std::ofstream ofs(output_filename, std::ios::binary);
    ofs.write(reinterpret_cast<char*>(dictionary.data()), sizeof(T) * dict_size);
    ofs.close();
}

template<typename T>
void generate_dictionary(const char* input_filename,
                         const std::size_t sample_count,
                         const std::size_t sample_size,
                         const char* output_filename) {
    const std::size_t dict_size = sample_count * sample_size;

    std::ifstream ifs(input_filename, std::ios::binary);
    const auto begin = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    const std::size_t file_length = end - begin;
    ifs.seekg(0);
    const std::size_t text_length = file_length / sizeof(T);

    const std::vector<std::size_t> sampling_pos_set = generate_sampling_position_set(sample_count, sample_size, text_length);
    std::vector<T> dictionary(dict_size, 0);
    fill_dictionary(dictionary, ifs, sampling_pos_set, sample_size);
    ifs.close();

    output_dictionary<T>(output_filename, dictionary, dict_size);
}
