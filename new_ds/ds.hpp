#pragma once

#include <istream>
#include <ostream>
#include <vector>

#include <cstdint>

#include <sdsl/bit_vectors.hpp>
#include <sdsl/int_vector.hpp>

struct ds {
    sdsl::bit_vector dense_container;
    sdsl::int_vector<> dense_starts;

    sdsl::int_vector<> sparse_container;
    sdsl::int_vector<> sparse_starts;

    sdsl::bit_vector subset_container;
    sdsl::int_vector<> subset_starts;
    sdsl::int_vector<> ancestor_ptrs;

    ds(const sdsl::bit_vector& dense_container,
       const sdsl::int_vector<>& dense_starts,
       const sdsl::int_vector<>& sparse_container,
       const sdsl::int_vector<>& sparse_starts,
       const sdsl::bit_vector& subset_container,
       const sdsl::int_vector<>& subset_starts,
       const sdsl::int_vector<>& ancestor_ptrs)
        : dense_container(dense_container),
          dense_starts(dense_starts),
          sparse_container(sparse_container),
          sparse_starts(sparse_starts),
          subset_container(subset_container),
          subset_starts(subset_starts),
          ancestor_ptrs(ancestor_ptrs) {}

    std::int64_t size_in_bytes() const {
        return sdsl::size_in_bytes(dense_container)
            + sdsl::size_in_bytes(dense_starts)
            + sdsl::size_in_bytes(sparse_container)
            + sdsl::size_in_bytes(sparse_starts)
            + sdsl::size_in_bytes(subset_container)
            + sdsl::size_in_bytes(subset_starts)
            + sdsl::size_in_bytes(ancestor_ptrs);
    }

    std::int64_t serialize(std::ostream& os) const {
        std::int64_t bytes_written = 0;

        bytes_written += dense_container.serialize(os);
        bytes_written += dense_starts.serialize(os);

        bytes_written += sparse_container.serialize(os);
        bytes_written += sparse_starts.serialize(os);

        bytes_written += subset_container.serialize(os);
        bytes_written += subset_starts.serialize(os);
        bytes_written += ancestor_ptrs.serialize(os);

        return bytes_written;
    }

    void load(std::istream& is) {
        dense_container.load(is);
        dense_starts.load(is);

        sparse_container.load(is);
        sparse_starts.load(is);

        subset_container.load(is);
        subset_starts.load(is);
        ancestor_ptrs.load(is);
    };

    bool is_root(const std::int64_t idx) const {
        return idx < (dense_starts.size() - 1 + sparse_starts.size() - 1);
    }

    bool is_subset(const std::int64_t idx) const {
        return !is_root(idx);
    }

    bool is_dense(const std::int64_t idx) const {
        return (idx < dense_starts.size() - 1);
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
        std::vector<std::uint32_t> s;

        const std::int64_t beg = dense_starts[idx];
        const std::int64_t end = dense_starts[idx + 1];
        const std::int64_t sz = end - beg;

        for (std::int64_t i = 0; i < sz; ++i) {
            if (dense_container[beg + i]) {
                s.push_back(i);
            }
        }

        return s;
    }

    std::vector<std::uint32_t> extract_sparse(const std::int64_t idx) const {
        std::vector<std::uint32_t> s;

        const std::int64_t beg = sparse_starts[idx];
        const std::int64_t end = sparse_starts[idx + 1];
        const std::int64_t sz = end - beg;

        for (std::int64_t i = 0; i < sz; ++i) {
            s.push_back(sparse_container[beg + i]);
        }

        return s;
    }

    std::vector<std::uint32_t> extract_subset(const std::int64_t idx) const {
        std::int64_t anc_idx = ancestor_ptrs[idx];

        if (is_dense(anc_idx)) {
            anc_idx = dense_idx(anc_idx);
            const auto as = extract_dense(anc_idx);

            const std::int64_t beg = subset_starts[idx];
            const std::int64_t end = subset_starts[idx + 1];
            const std::int64_t sz = end - beg;

            std::vector<std::uint32_t> s;

            for (std::int64_t i = 0; i < sz; ++i) {
                if (subset_container[beg + i]) {
                    s.push_back(as[i]);
                }
            }

            return s;
        } else if (is_sparse(anc_idx)) {
            anc_idx = sparse_idx(anc_idx);
            // optimize to use sparse array directly
            const auto as = extract_sparse(anc_idx);

            const std::int64_t beg = subset_starts[idx];
            const std::int64_t end = subset_starts[idx + 1];
            const std::int64_t sz = end - beg;

            std::vector<std::uint32_t> s;

            for (std::int64_t i = 0; i < sz; ++i) {
                if (subset_container[beg + i]) {
                    s.push_back(as[i]);
                }
            }

            return s;
        } else {
            const std::int64_t beg = subset_starts[idx];
            const std::int64_t end = subset_starts[idx + 1];
            auto bv = make_helper_bv(beg, end);

            while (is_subset(anc_idx)) {
                anc_idx = subset_idx(anc_idx);

                const std::int64_t anc_beg = subset_starts[anc_idx];
                const std::int64_t anc_end = subset_starts[anc_idx + 1];
                auto anc_bv = make_helper_bv(anc_beg, anc_end);

                bv = extract_bits(anc_bv, bv);
                anc_idx = ancestor_ptrs[anc_idx];
            }

            if (is_dense(anc_idx)) {
                anc_idx = dense_idx(anc_idx);
                const auto as = extract_dense(anc_idx);

                std::vector<std::uint32_t> s;

                for (std::int64_t i = 0; i < as.size(); ++i) {
                    if (bv[i]) {
                        s.push_back(as[i]);
                    }
                }

                return s;
            } else {
                anc_idx = sparse_idx(anc_idx);
                const auto as = extract_sparse(anc_idx);

                std::vector<std::uint32_t> s;

                for (std::int64_t i = 0; i < as.size(); ++i) {
                    if (bv[i]) {
                        s.push_back(as[i]);
                    }
                }

                return s;
            }
        }
    }

    std::vector<bool> make_helper_bv(const std::int64_t beg,
                                     const std::int64_t end) const {
        const std::int64_t sz = end - beg;
        std::vector<bool> bv(sz, 0);

        for (std::int64_t i = 0; i < sz; ++i) {
            bv[i] = subset_container[beg + i];
        }

        return bv;
    }

    std::vector<bool> extract_bits(const std::vector<bool>& src,
                                   const std::vector<bool>& mask) const {
        const std::int64_t sz = src.size();
        std::vector<bool> dst(sz, 0);

        for (std::int64_t m = 0, k = 0; m < sz; ++m) {
            if (src[m]) {
                dst[m] = mask[k++];
            }
        }

        return dst;
    }
};
