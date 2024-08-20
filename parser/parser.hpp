#pragma once

#include <fstream>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

template<typename T>
std::vector<T> read_file(const char* filename) {
    std::ifstream ifs(filename, std::ios::binary);

    const auto begin = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    const std::size_t len = (end - begin) / sizeof(T);
    ifs.seekg(0);

    std::vector<T> v(len, 0);

    for (std::size_t i = 0; i < len; ++i) {
        ifs.read(reinterpret_cast<char*>(v.data() + i), sizeof(T));
    }

    ifs.close();

    return v;
}

template<typename T1, typename T2>
inline std::optional<std::size_t> binarySearchLB(const std::vector<T1>& ref, const std::vector<T2>& sa,
                                                 const std::size_t lo, const std::size_t hi,
                                                 const std::size_t offset, const T1 c) {
    std::size_t low = lo;
    std::size_t high = hi;

    while (low <= high) {
        const std::size_t mid = (low + high) >> 1;
        const auto midVal = ref.at(sa.at(mid) + offset);

        if (midVal < c) {
            low = mid + 1;
        } else if (midVal > c) {
            high = mid - 1;
        } else { //midVal == c
            if (mid == lo) {
                return mid; // leftmost occ of key found
            }

            const auto midValLeft = ref.at(sa.at(mid - 1) + offset);
            if (midValLeft == midVal) {
                high = mid - 1; //discard mid and the ones to the right of mid
            } else { //midValLeft must be less than midVal == c
                return mid; //leftmost occ of key found
            }
        }
    }

    return {}; // key not found.
}

template<typename T1, typename T2>
inline std::optional<std::size_t> binarySearchRB(const std::vector<T1>& ref, const std::vector<T2>& sa,
                                                 const std::size_t lo, const std::size_t hi,
                                                 const std::size_t offset, const T1 c) {
    std::size_t low = lo;
    std::size_t high = hi;

    while (low <= high) {
        const std::size_t mid = (low + high) >> 1;
        const auto midVal = ref.at(sa.at(mid) + offset);

        if (midVal < c) {
            low = mid + 1;
        } else if (midVal > c) {
            high = mid - 1;
        } else { //midVal == c
            if (mid == hi) {
                return mid; // rightmost occ of key found
            }

            const auto midValRight = ref.at(sa.at(mid + 1) + offset);
            if (midValRight == midVal) {
                low = mid + 1; //discard mid and the ones to the left of mid
            } else { //midValRight must be greater than midVal == c
                return mid; //rightmost occ of key found
            }
        }
    }
    return {}; // key not found.
}

template<typename T1, typename T2>
std::tuple<std::size_t, std::size_t> computeLZFactorAt(const std::vector<T1>& input,
                                                       const std::vector<T1>& ref,
                                                       const std::vector<T2>& sa,
                                                       const std::size_t input_pos) {
    std::size_t offset = 0;
    std::size_t j = input_pos;

    std::size_t match = 0;
    std::size_t nlb = 0;
    std::size_t nrb = ref.size() - 1;

    while (j < input.size()) {
        if (nlb == nrb) {
            if (ref[sa[nlb] + offset] != input[j]) {
                break;
            }
        }
        else {
            if (const auto opt = binarySearchLB<T1, T2>(ref, sa, nlb, nrb, offset, input.at(j))) {
                nlb = opt.value();
            } else {
                break;
            }

            if (const auto opt = binarySearchRB<T1, T2>(ref, sa, nlb, nrb, offset, input.at(j))) {
                nrb = opt.value();
            } else {
                break;
            }
        }

        match = sa[nlb];
        ++j;
        ++offset;
    }

    return {match, offset};
}

template<typename T1, typename T2>
std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> lzFactorize(const std::vector<T1>& input,
                                                                           const std::vector<T1>& ref,
                                                                           const std::vector<T2>& sa) {
    std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> spl_vec;
    std::size_t i = 0;

    while (i < input.size()) {
        auto [pos, len] = computeLZFactorAt<T1, T2>(input, ref, sa, i);

        if (len <= 1) {
            pos = static_cast<std::size_t>(input.at(i));
            len = 1;
        }

        spl_vec.push_back({i, pos, len});

        i += len;
    }

    return spl_vec;
}
