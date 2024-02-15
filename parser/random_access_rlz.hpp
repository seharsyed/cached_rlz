#pragma once

#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <cstdint>

#include <sdsl/sd_vector.hpp>

template<typename T>
struct random_access_rlz {
    sdsl::sd_vector<> starts;
    sdsl::rank_support_sd<> starts_rs;
    sdsl::select_support_sd<> starts_ss;
    std::vector<T> ref_vec;
    std::vector<std::size_t> ref_ptrs;

    random_access_rlz(const std::vector<T>& ref_vec,
                      const std::vector<std::tuple<std::size_t, std::size_t, std::size_t>>& spl_vec) : ref_vec(ref_vec) {
        const auto [last_start, last_pos, last_len] = spl_vec.back();
        const std::size_t decompressed_sz = last_start + last_len;
        const std::size_t phrases = spl_vec.size();

        sdsl::sd_vector_builder starts_builder(decompressed_sz + 1, phrases + 1);
        ref_ptrs.reserve(spl_vec.size());
        for (const auto [start, pos, len] : spl_vec) {
            starts_builder.set(start);
            ref_ptrs.push_back(pos);
        }
        starts_builder.set(decompressed_sz);

        starts = sdsl::sd_vector<>(starts_builder);
        sdsl::util::init_support(this->starts_rs, &(this->starts));
        sdsl::util::init_support(this->starts_ss, &(this->starts));
    }

    std::size_t length_cumulative_sum(const std::size_t phrase) const {
        return starts_ss.select(phrase + 1);
    }

    std::size_t pos_to_phrase(const std::size_t pos) const {
        return starts_rs.rank(pos + 1) - 1;
    }

    std::size_t phrase_length(const std::size_t phrase) const {
        return length_cumulative_sum(phrase + 1) - length_cumulative_sum(phrase);
    }

    std::size_t phrase_begin(const std::size_t phrase) const {
        return length_cumulative_sum(phrase);
    }

    std::size_t phrase_end(const std::size_t phrase) const {
        return length_cumulative_sum(phrase + 1);
    }

    std::size_t pos_to_pos_in_ref(const std::size_t pos) const {
        const auto phrase = pos_to_phrase(pos);
        return ref_ptrs[phrase] + pos - length_cumulative_sum(phrase);
    }

    std::size_t length_until_phrase_end(const std::size_t pos) const {
        const auto phrase = pos_to_phrase(pos);
        return length_cumulative_sum(phrase + 1) - pos;
    }

    T access(const std::size_t pos) const {
        const auto phrase = pos_to_phrase(pos);
        if (phrase_length(phrase) > 1) {
            return ref_vec[ref_ptrs[phrase] + pos - length_cumulative_sum(phrase)];
        } else {
            return static_cast<T>(ref_ptrs[phrase]);
        }
    }

    void get(const std::size_t pos, const std::size_t len, std::vector<T>& buf) const {
        for (std::size_t i = 0; i < len; ++i) {
            buf.push_back(access(pos + i));
        }
    }

    void get_spans(std::size_t pos, std::size_t len, std::vector<std::span<const T>>& buf) const {
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

    std::size_t size_in_bytes() const {
        return sizeof(T) * ref_vec.size()
            + sizeof(std::size_t) * ref_ptrs.size()
            + starts.size() / 8;
    }
};
