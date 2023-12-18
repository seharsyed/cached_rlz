#pragma once

#include <span>
#include <tuple>
#include <vector>

#include <cstdint>

#include <sdsl/sd_vector.hpp>

template<typename T>
struct random_access_rlz {
    sdsl::sd_vector<> starts;
    sdsl::rank_support_sd<> starts_rs;
    sdsl::select_support_sd<> starts_ss;

    std::vector<T> ref_vec;

    std::vector<std::int64_t> relative_ref_ptrs;

    std::vector<T> mismatch_symbol_vec;
    std::vector<bool> mismatch;
    std::vector<std::int64_t> mismatch_prefix_sums;

    random_access_rlz(const std::vector<T>& ref_vec,
                      const std::vector<std::tuple<std::size_t, std::size_t, std::size_t>>& spl_vec) : ref_vec(ref_vec) {
        const auto [last_start, last_pos, last_len] = spl_vec.back();
        const std::int64_t decompressed_sz = last_start + last_len;
        const std::int64_t phrases = spl_vec.size();

        sdsl::sd_vector_builder starts_builder(decompressed_sz + 1, phrases + 1);

        mismatch.assign(decompressed_sz, 0);

        for (const auto& [start, pos, len] : spl_vec) {
            starts_builder.set(start);

            if (len == 0) {
                mismatch[start] = 1;
                mismatch_symbol_vec.push_back(static_cast<T>(pos ^ (1ull << 63ull)));
            }
        }
        starts_builder.set(decompressed_sz);

        starts = sdsl::sd_vector<>(starts_builder);
        sdsl::util::init_support(this->starts_rs, &(this->starts));
        sdsl::util::init_support(this->starts_ss, &(this->starts));

        mismatch_prefix_sums.assign(decompressed_sz, 0);
        for (std::int64_t i = 1; i < decompressed_sz; ++i) {
            this->mismatch_prefix_sums[i] = this->mismatch_prefix_sums[i - 1] + (this->mismatch[i - 1] ? 1 : 0);
        }

        relative_ref_ptrs.assign(spl_vec.size(), 0);
        relative_ref_ptrs[0] = std::get<1>(spl_vec[0]);
        for (std::int64_t i = 1; i < spl_vec.size(); ++i) {
            relative_ref_ptrs[i] = std::get<1>(spl_vec[i]) - length_cumulative_sum(i);
        }
    }

    std::size_t length_cumulative_sum(const std::int64_t phrase) const {
        return starts_ss.select(phrase + 1);
    }

    std::size_t pos_to_phrase(const std::int64_t pos) const {
        return starts_rs.rank(pos + 1) - 1;
    }
    std::size_t phrase_length(const std::int64_t phrase) const {
        return length_cumulative_sum(phrase + 1) - length_cumulative_sum(phrase);
    }

    std::size_t phrase_begin(const std::int64_t phrase) const {
        return length_cumulative_sum(phrase);
    }

    std::size_t phrase_end(const std::int64_t phrase) const {
        return length_cumulative_sum(phrase + 1);
    }

    std::size_t pos_to_pos_in_ref(const std::int64_t pos) const {
        return relative_ref_ptrs[pos_to_phrase(pos)] + pos;
    }

    std::int64_t length_until_phrase_end(const std::int64_t pos) const {
        const auto phrase = pos_to_phrase(pos);

        return length_cumulative_sum(phrase + 1) - pos;
    }

    T access(const std::int64_t pos) const {
        if (mismatch[pos]) {
            return mismatch_symbol_vec[mismatch_prefix_sums[pos]];
        }
        else {
            return ref_vec[relative_ref_ptrs[pos_to_phrase(pos)] + pos];
        }
    }

    void get(const std::int64_t pos, const std::int64_t len, std::vector<T>& buf) const {
        for (std::size_t i = 0; i < len; ++i) {
            buf.push_back(access(pos + i));
        }
    }

    void get_spans(std::int64_t pos, std::int64_t len, std::vector<std::span<const T>>& buf) const {
        std::int64_t copied = 0;
        while (copied < len) {
            std::int64_t to_copy = length_until_phrase_end(pos);

            if (to_copy > len - copied) {
                to_copy = len - copied;
            }

            std::span<const T> sp(ref_vec.data() + pos_to_pos_in_ref(pos), to_copy);
            buf.push_back(sp);
            copied += to_copy;
            pos += to_copy;

        }

    }
};
