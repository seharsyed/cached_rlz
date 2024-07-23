#pragma once

#include <algorithm>
#include <iterator>

template<typename T>
T median5(const T a, const T b, const T c, const T d, const T e) {
    T arr[5] = {a, b, c, d, e};
    std::sort(arr, arr + 5);
    return arr[2];
}

template<typename T>
void quicksort(const std::size_t l, const std::size_t r, T* data) {
    constexpr std::size_t SORT_THRESHOLD = 64;
    constexpr std::size_t TASK_THRESHOLD = 4096;

    if (l < r) {
        if (r - l < SORT_THRESHOLD) {
            std::sort(data + l, data + r);
        } else {
            const std::size_t m1 = l + (r - l) / 4;
            const std::size_t m2 = l + (r - l) / 2;
            const std::size_t m3 = l + 3 * (r - l) / 4;
            const auto x = median5(data[l], data[m1], data[m2], data[m3], data[r - 1]);

            std::size_t i = l;
            std::size_t a = l;
            std::size_t b = r - 1;
            while (i <= b) {
                if (data[i] < x) {
                    std::swap(data[i++], data[a++]);
                } else if (data[i] > x) {
                    std::swap(data[i], data[b--]);
                } else {
                    ++i;
                }
            }

            #pragma omp taskgroup
            {
                #pragma omp task shared(data) untied if (r - l > TASK_THRESHOLD)
                quicksort(l, a, data);
                #pragma omp task shared(data) untied if (r - l > TASK_THRESHOLD)
                quicksort(b, r, data);
                #pragma omp taskyield
            }
        }
    }
}
