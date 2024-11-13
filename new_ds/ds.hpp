#pragma once

#include <istream>
#include <map>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

#include <cstdint>

#include <sdsl/bit_vectors.hpp>
#include <sdsl/bits.hpp>
#include <sdsl/int_vector.hpp>

struct ds {
    sdsl::bit_vector dense_container;
    sdsl::int_vector<> dense_starts;

    sdsl::int_vector<> sparse_container;
    sdsl::int_vector<> sparse_starts;

    sdsl::bit_vector subset_container;
    sdsl::int_vector<> subset_starts;
    sdsl::int_vector<> parent_vec;

    ds() {}

    ds(const sdsl::bit_vector& dense_container,
       const sdsl::int_vector<>& dense_starts,
       const sdsl::int_vector<>& sparse_container,
       const sdsl::int_vector<>& sparse_starts,
       const sdsl::bit_vector& subset_container,
       const sdsl::int_vector<>& subset_starts,
       const sdsl::int_vector<>& parent_vec)
        : dense_container(dense_container),
          dense_starts(dense_starts),
          sparse_container(sparse_container),
          sparse_starts(sparse_starts),
          subset_container(subset_container),
          subset_starts(subset_starts),
          parent_vec(parent_vec) {}

    std::int64_t size_in_bytes() const {
        return sdsl::size_in_bytes(dense_container)
            + sdsl::size_in_bytes(dense_starts)
            + sdsl::size_in_bytes(sparse_container)
            + sdsl::size_in_bytes(sparse_starts)
            + sdsl::size_in_bytes(subset_container)
            + sdsl::size_in_bytes(subset_starts)
            + sdsl::size_in_bytes(parent_vec);
    }

    std::int64_t serialize(std::ostream& os) const {
        std::int64_t bytes_written = 0;

        bytes_written += dense_container.serialize(os);
        bytes_written += dense_starts.serialize(os);

        bytes_written += sparse_container.serialize(os);
        bytes_written += sparse_starts.serialize(os);

        bytes_written += subset_container.serialize(os);
        bytes_written += subset_starts.serialize(os);
        bytes_written += parent_vec.serialize(os);

        return bytes_written;
    }

    void load(std::istream& is) {
        dense_container.load(is);
        dense_starts.load(is);

        sparse_container.load(is);
        sparse_starts.load(is);

        subset_container.load(is);
        subset_starts.load(is);
        parent_vec.load(is);
    };

    bool is_root(const std::int64_t idx) const {
        return idx < root_count();
    }

    bool is_subset(const std::int64_t idx) const {
        return !is_root(idx);
    }

    bool is_dense(const std::int64_t idx) const {
        return idx < dense_count();
    }

    bool is_sparse(const std::int64_t idx) const {
        return is_root(idx) && !is_dense(idx);
    }

    std::int64_t dense_count() const {
        return dense_starts.size() - 1;
    }

    std::int64_t sparse_count() const {
        return sparse_starts.size() - 1;
    }

    std::int64_t subset_count() const {
        return subset_starts.size() - 1;
    }

    std::int64_t root_count() const {
        return dense_count() + sparse_count();
    }

    std::int64_t size() const {
        return dense_count() + sparse_count() + subset_count();
    }

    std::int64_t dense_idx(const std::int64_t idx) const {
        return idx;
    }

    std::int64_t sparse_idx(const std::int64_t idx) const {
        return idx - dense_count();
    }

    std::int64_t subset_idx(const std::int64_t idx) const {
        return idx - root_count();
    }

    std::vector<std::uint32_t> extract(const std::int64_t idx) const {
        if (is_dense(idx)) {
            return extract_dense(dense_idx(idx));
        } else if (is_sparse(idx)) {
            return extract_sparse(sparse_idx(idx));
        } else {
            return extract_subset(subset_idx(idx));
        }
    }

    std::vector<std::uint32_t> extract_dense(const std::int64_t idx) const {
        const std::size_t beg = dense_starts[idx];
        const std::size_t end = dense_starts[idx + 1];
        const std::size_t sz = end - beg;

        // TODO: optimize to use popcount
        std::size_t elems = 0;
        for (std::size_t i = 0; i < sz; ++i) {
            if (dense_container[beg + i]) {
                ++elems;
            }
        }

        std::vector<std::uint32_t> s(elems);
        for (std::size_t i = 0, j = 0; i < sz; ++i) {
            if (dense_container[beg + i]) {
                s[j++] = i;
            }
        }

        return s;
    }

    std::vector<std::uint32_t> extract_sparse(const std::int64_t idx) const {
        const std::size_t beg = sparse_starts[idx];
        const std::size_t end = sparse_starts[idx + 1];
        const std::size_t sz = end - beg;

        std::vector<std::uint32_t> s(sz);
        for (std::size_t i = 0; i < sz; ++i) {
            s[i] = sparse_container[beg + i];
        }

        return s;
    }

    std::vector<std::uint32_t> extract_subset(const std::int64_t idx) const {
        static std::stack<std::int64_t> st;
        st.push(idx);
        std::int64_t parent = parent_vec[idx];
        while (is_subset(parent)) {
            parent = subset_idx(parent);
            st.push(parent);
            parent = parent_vec[parent];
        }

        std::size_t beg = 0;
        std::size_t end = 0;
        std::size_t sz = 0;

        if (is_dense(parent)) {
            const auto root = dense_idx(parent);
            beg = dense_starts[root];
            end = dense_starts[root + 1];
            sz = end - beg;
        } else {
            const auto root = sparse_idx(parent);
            beg = sparse_starts[root];
            end = sparse_starts[root + 1];
            sz = sparse_container[end - 1] + 1;
        }

        sdsl::bit_vector bv(sz, 0);

        if (is_dense(parent)) {
            for (std::size_t i = 0; i < (end - beg); ++i) {
                bv[i] = dense_container[beg + i];
            }
        } else {
            for (std::size_t i = 0; i < (end - beg); ++i) {
                bv[sparse_container[beg + i]] = 1;
            }
        }

        while (st.size()) {
            const auto ss = st.top(); st.pop();
            const auto ss_beg = subset_starts[ss];
            const auto ss_end = subset_starts[ss + 1];
            const auto words = (bv.size() + 63) / 64;

            for (std::size_t w = 0, elem = ss_beg; w < words; ++w) {
                const std::uint64_t bits = std::popcount(bv.data()[w]);
                std::uint64_t mask = ~0ull;
                std::uint64_t temp = 0ull;
                for (std::uint64_t b = 1; (b <= bits); ++b) {
                    const std::uint64_t idx = std::countr_zero(bv.data()[w] & mask);
                    const std::uint64_t bit = subset_container[elem++];
                    temp |= (bit << idx);
                    mask &= ~(1ull << idx);
                }
                bv.data()[w] = temp;
            }
        }

        const auto words = (bv.size() + 63) / 64;
        std::size_t elems = 0;
        for (std::size_t w = 0; w < words; ++w) {
            elems += std::popcount(bv.data()[w]);
        }

        std::vector<std::uint32_t> s(elems);
        for (std::size_t i = 0, j = 0; i < bv.size(); ++i) {
            if (bv[i]) {
                s[j++] = i;
            }
        }

        return s;
    }

    std::map<std::string, std::int64_t> space_breakdown() const {
       return std::map<std::string, std::int64_t>{
            {"dense_container", sdsl::size_in_bytes(dense_container)},
            {"dense_starts", sdsl::size_in_bytes(dense_starts)},
            {"sparse_container", sdsl::size_in_bytes(sparse_container)},
            {"sparse_starts", sdsl::size_in_bytes(sparse_starts)},
            {"subset_container", sdsl::size_in_bytes(subset_container)},
            {"subset_starts", sdsl::size_in_bytes(subset_starts)},
            {"parent_vec", sdsl::size_in_bytes(parent_vec)}
        };
    }
};
