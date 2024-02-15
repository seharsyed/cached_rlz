#pragma once

#include "parser.hpp"

template<typename T1, typename T2>
std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> lzFactorizeColors(const char* input_filename,
                                                                                 const T1* ref, const std::size_t ref_sz,
                                                                                 const T2* sa) {
    std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> spl_vec;

    std::ifstream ifs(input_filename, std::ios::binary);
    for (; !ifs.eof(); ifs.peek()) {
        T1 color_set_sz = 0;
        ifs.read(reinterpret_cast<char*>(&color_set_sz), sizeof(T1));
        std::vector<T1> color_set(color_set_sz, 0);
        ifs.read(reinterpret_cast<char*>(color_set.data()), sizeof(T1) * color_set_sz);

        const auto res_vec = lzFactorize(color_set.data(), color_set.size(), ref, ref_sz, sa);
        spl_vec.insert(spl_vec.end(), res_vec.begin(), res_vec.end());
    }
    ifs.close();

    return spl_vec;
}
