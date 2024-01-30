#pragma once

#include <fstream>
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
inline std::int64_t binarySearchLB(const T1* ref, const T2* sa,
                                   std::int64_t lo, std::int64_t hi, const std::int64_t offset, const T1 c) {
    std::int64_t low = lo, high = hi;
    while (low <= high) {
        std::int64_t mid = (low + high) >> 1;
        T1 midVal = ref[sa[mid] + offset];
        if (midVal < c)
            low = mid + 1;
        else if (midVal > c)
            high = mid - 1;
        else { //midVal == c
            if (mid == lo)
                return mid; // leftmost occ of key found
            T1 midValLeft = ref[sa[mid - 1] + offset];
            if (midValLeft == midVal) {
                high = mid - 1; //discard mid and the ones to the right of mid
            } else { //midValLeft must be less than midVal == c
                return mid; //leftmost occ of key found
            }
        }
    }
    return -(low + 1);  // key not found.
}

template<typename T1, typename T2>
inline std::int64_t binarySearchRB(const T1* ref, const T2* sa,
                                   std::int64_t lo, std::int64_t hi, const std::int64_t offset, const T1 c) {
    std::int64_t low = lo, high = hi;
    while (low <= high) {
        std::int64_t mid = (low + high) >> 1;
        T1 midVal = ref[sa[mid] + offset];
        if (midVal < c)
            low = mid + 1;
        else if (midVal > c)
            high = mid - 1;
        else { //midVal == c
            if (mid == hi)
                return mid; // rightmost occ of key found
            T1 midValRight = ref[sa[mid + 1] + offset];
            if (midValRight == midVal) {
                low = mid + 1; //discard mid and the ones to the left of mid
            } else { //midValRight must be greater than midVal == c
                return mid; //rightmost occ of key found
            }
        }
    }
    return -(low + 1);  // key not found.
}

template<typename T1, typename T2>
std::pair<std::size_t, std::size_t> computeLZFactorAt(const T1* input, const std::size_t input_sz,
                                                      const T1* ref, const std::size_t ref_sz,
                                                      const T2* sa, const std::size_t input_pos) {
    std::size_t offset = 0;
    std::size_t j = input_pos;

    std::size_t match = 0;
    std::int64_t nlb = 0;
    std::int64_t nrb = ref_sz - 1;

    while (j < input_sz) {
        if (nlb == nrb) {
            if (ref[sa[nlb] + offset] != input[j]) {
                break;
            }
        }
        else {
            nlb = binarySearchLB<T1, T2>(ref, sa, nlb, nrb, offset, input[j]);
            if (nlb < 0) {
                break;
            }
            nrb = binarySearchRB<T1, T2>(ref, sa, nlb, nrb, offset, input[j]);
        }

        match = sa[nlb];
        ++j;
        ++offset;
    }

    return std::make_pair(match, offset);
}

template<typename T1, typename T2>
std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> lzFactorize(const T1* input, const std::size_t input_sz,
                                                                           const T1* ref, const std::size_t ref_sz,
                                                                           const T2* sa) {
    std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> spl_vec;
    std::size_t i = 0;

    while (i < input_sz) {
        auto [pos, len] = computeLZFactorAt<T1, T2>(input, input_sz, ref, ref_sz, sa, i);

        if (len == 0) {
            pos = static_cast<std::size_t>(input[i]) | (1ull << 63ull);
        }

        spl_vec.push_back({i, pos, len});

        if (len == 0) {
            ++i;
        }
        else {
            i += len;
        }
    }

    return spl_vec;
}
