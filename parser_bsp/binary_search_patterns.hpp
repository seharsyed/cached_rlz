#pragma once

#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include <cstdint>
#include <cstring>

#ifndef CACHE_LINE
// Apparently the most common cache line size is 64.
#define CACHE_LINE 64
#endif

class control_binary_search {
   private:
    uint64_t const* data_;
    const uint64_t n_;

   public:
    control_binary_search(uint64_t const* data, uint64_t n)
        : data_(data), n_(n) {}

    std::pair<uint64_t, uint64_t> find(uint64_t q) const {
        uint64_t a = 0;
        uint64_t b = n_ - 1;
        while (a < b) {
            uint64_t m = (a + b + 1) / 2;
            if (data_[m] < q) {
                a = m;
            } else {
                b = m - 1;
            }
        }
        return {data_[a], a};
    }

    uint64_t bytes() {
        return sizeof(control_binary_search) + n_ * sizeof(uint64_t);
    }
};

template <uint64_t block_size = 1024>
class b_star_blocks {
   private:
    static_assert(__builtin_popcountll(block_size) == 1);
    static_assert(block_size <= 1024);
    static_assert(block_size >= 2);

    class node {
       public:
        typedef std::pair<uint64_t, uint64_t> item;
        item children[block_size];

       private:
        template <uint16_t size>
        static item branch(const item* arr, uint64_t q) {
            if constexpr (size == 2) {
                return arr[1].first < q ? arr[1] : arr[0];
            }
            bool gr = arr[size / 2].first < q;
            return branch<size / 2>(arr + (gr * (size / 2)), q);
        }

       public:
        node() : children() {
            std::fill_n(children, block_size, item(~uint64_t(0), 0));
        }

        node(const node& other) : children() {
            std::copy(other.children, other.children + block_size, children);
        }

        node& operator=(const node& other) {
            std::copy(other.children, other.children + block_size, children);
            return *this;
        }

        uint64_t build(uint64_t& i_pos, uint64_t& c_idx, uint64_t n,
                       std::vector<uint64_t>& n_children, node* nd_arr,
                       uint64_t* data) {
            uint64_t nn = n_children.back();
            n_children.pop_back();
            if (n_children.size() == 0) {
                for (uint64_t i = 0; i < nn && i < block_size; i++) {
                    children[i] = {data[c_idx], c_idx};
                    c_idx++;
                }
            } else {
                for (uint64_t i = 0; i < nn && i < block_size; i++) {
                    nd_arr[i_pos] = {};
                    children[i] = {0, i_pos++};
                    uint64_t n_s = nd_arr[children[i].second].build(
                        i_pos, c_idx, n, n_children, nd_arr, data);
                    children[i].first = n_s;
                }
            }
            nn -= nn >= block_size ? block_size : nn;
            n_children.push_back(nn);
            return children[0].first;
        }

        item find(uint64_t q) const {
            constexpr uint64_t lines = CACHE_LINE / sizeof(item);
            for (uint64_t i = 0; i < block_size; i += lines) {
                __builtin_prefetch(children + i);
            }
            return branch<block_size>(children, q);
        }

        void print() {
            for (uint64_t i = 0; i < block_size; i++) {
                std::cout << children[i].first << ", " << children[i].second
                          << std::endl;
            }
        }
    };

    node* nodes_;
    uint64_t levels_;
    uint64_t node_count_;

   public:
    b_star_blocks() : levels_(1) {}

    b_star_blocks(uint64_t* data, uint64_t n) : levels_(1) {
        std::vector<uint64_t> c_per_level;
        c_per_level.push_back(n);
        node_count_ = 1;
        while (c_per_level.back() > block_size) {
            uint64_t nn = c_per_level.back();
            nn = nn / block_size + (nn % block_size ? 1 : 0);
            node_count_ += nn;
            c_per_level.push_back(nn);
            levels_++;
        }
        nodes_ = (node*)malloc(node_count_ * sizeof(node));
        uint64_t i_pos = 1;
        uint64_t c_idx = 0;
        nodes_[0] = node();
        nodes_[0].build(i_pos, c_idx, n, c_per_level, nodes_, data);
    }

    ~b_star_blocks() {
        free(nodes_);
    }

    void build(uint64_t* data, uint64_t n) {
        std::vector<uint64_t> c_per_level;
        c_per_level.push_back(n);
        node_count_ = 1;
        while (c_per_level.back() > block_size) {
            uint64_t nn = c_per_level.back();
            nn = nn / block_size + (nn % block_size ? 1 : 0);
            node_count_ += nn;
            c_per_level.push_back(nn);
            levels_++;
        }
        nodes_ = (node*)malloc(node_count_ * sizeof(node));
        uint64_t i_pos = 1;
        uint64_t c_idx = 0;
        nodes_[0] = node();
        nodes_[0].build(i_pos, c_idx, n, c_per_level, nodes_, data);
    }

    std::pair<uint64_t, uint64_t> find(uint64_t q) const {
        std::pair<uint64_t, uint64_t> ret = {0, 0};
        for (uint64_t i = 0; i < levels_; i++) {
            ret = nodes_[ret.second].find(q);
        }
        return ret;
    }

    void print() {
        std::cout << "B-star search tree with " << node_count_ << " blocks"
                  << std::endl;
        for (uint64_t i = 0; i < node_count_; i++) {
            std::cout << " node " << i << std::endl;
            nodes_[i].print();
        }
    }

    uint64_t bytes() {
        return sizeof(b_star_blocks) + node_count_ * sizeof(node);
    }
};

class heap_order_search {
   public:
    std::vector<uint64_t> items_;
    uint64_t n_;

    heap_order_search() : items_(), n_(n - 1) {}

    heap_order_search(uint64_t* data, uint64_t n) : items_(), n_(n - 1) {
        items_.push_back(data[n / 2]);
        build(0, 0, n - 1, data, false);
    }

    std::pair<uint64_t, uint64_t> find(uint64_t q) const {
        uint64_t a = 0;
        uint64_t b = n_;
        uint64_t i = 0;
        bool stepped_right = false;
        while (a + 1 < b) {
            uint64_t t = (a + b + 1) / 2;
            if (items_[i] >= q) {
                b = t - 1;
                i = i * 2 + 1;
                stepped_right = false;
            } else {
                a = t;
                i = i * 2 + 2;
                stepped_right = true;
            }
        }
        if (a == b) [[unlikely]] {
            return {items_[i], a};
        }
        if (items_[i] >= q) [[likely]] {
            return {items_[stepped_right ? (i - 2) / 2 : i * 2 + 1], a};
        } else {
            return {items_[i], b};
        }
    }

    void build(uint64_t* data, uint64_t n) {
        items_.push_back(data[n / 2]);
        build(0, 0, n - 1, data, false);
    }

    void print() {
        std::cout << "Heap order search tree with " << items_.size()
                  << " nodes";
        for (uint64_t i = 0; i < items_.size(); i++) {
            if (i % 8 == 0) {
                std::cout << std::endl;
            }
            std::cout << items_[i] << "\t";
        }
        if ((items_.size() - 1) % 8 != 0) {
            std::cout << std::endl;
        }
    }

    uint64_t bytes() {
        return sizeof(heap_order_search) + items_.size() * sizeof(uint64_t);
    }

   private:
    void build(uint64_t idx, uint64_t a, uint64_t b, uint64_t* data,
               bool stepped_right) {
        if (a == b) {
            items_[idx] = data[a];
            return;
        }
        if (a + 1 == b) {
            items_[idx] = data[b];
            uint64_t n_idx = idx * 2 + 1;
            if (!stepped_right) {
                if (items_.size() < n_idx + 1) {
                    items_.resize(n_idx + 1);
                }
                build(n_idx, a, a, data, false);
            }
        } else {
            uint64_t t = (a + b + 1) / 2;
            items_[idx] = data[t];
            uint64_t n_idx = idx * 2 + 1;
            if (items_.size() < n_idx + 2) {
                items_.resize(n_idx + 2);
            }
            build(n_idx, a, t - 1, data, false);
            build(n_idx + 1, t, b, data, true);
        }
    }
};

template <uint64_t block_size = 64>
class b_heap_search {
   private:
    static_assert(__builtin_popcountll(block_size) == 1);
    static_assert(block_size <= 1024);
    static_assert(block_size >= 2);

    class node {
       public:
        typedef std::pair<uint64_t, uint64_t> item;
        uint64_t children[block_size];

       private:
        template <uint16_t size>
        static item branch(const uint64_t* arr, uint64_t q) {
            if constexpr (size == 2) {
                return arr[1] < q ? item(arr[1], 1) : item(arr[0], 0);
            }
            uint64_t offset = (arr[size / 2] < q) * (size / 2);
            item res = branch<size / 2>(arr + offset, q);
            return {res.first, offset + res.second};
        }

       public:
        node() : children() { std::fill_n(children, block_size, ~uint64_t(0)); }

        node(const node& other) : children() {
            std::copy(other.children, other.children + block_size, children);
        }

        node& operator=(const node& other) {
            std::copy(other.children, other.children + block_size, children);
            return *this;
        }

        item find(uint64_t q) const {
            constexpr uint64_t lines = CACHE_LINE / sizeof(uint64_t);
            for (uint64_t i = 0; i < block_size; i += lines) {
                __builtin_prefetch(children + i);
            }
            return branch<block_size>(children, q);
        }

        void print() {
            for (uint64_t i = 0; i < block_size; i++) {
                std::cout << children[i] << (i + 1 < block_size ? ", " : "");
            }
            std::cout << std::endl;
        }
    };

    node* nodes_;
    uint64_t levels_;
    uint64_t node_count_;

   public:
    b_heap_search() : levels_(1) {}

    b_heap_search(uint64_t* data, uint64_t n) : levels_(1) {
        uint64_t nn = n / block_size + (n % block_size ? 1 : 0);
        uint64_t leaves = nn;
        while (nn > block_size) {
            nn = nn / block_size + (nn % block_size ? 1 : 0);
            levels_++;
        }
        uint64_t n_lev = 1;
        uint64_t t_nodes = n_lev;
        for (uint64_t i = 1; i < levels_; i++) {
            n_lev *= block_size;
            t_nodes += n_lev;
        }
        node_count_ = t_nodes + leaves;
        nodes_ = (node*)malloc((node_count_) * sizeof(node));
        std::fill_n(nodes_, node_count_, node());
        std::memcpy(reinterpret_cast<uint64_t*>(nodes_ + t_nodes), data, n * sizeof(uint64_t));
        for (uint64_t i = t_nodes - 1; i < t_nodes; i--) {
            for (uint64_t k = 0; k < block_size; k++) {
                uint64_t c_idx = i * block_size + k + 1;
                if (c_idx < node_count_) {
                    nodes_[i].children[k] =
                        nodes_[i * block_size + k + 1].children[0];
                }
            }
        }
    }

    ~b_heap_search() {
        free(nodes_);
    }

    void build(uint64_t* data, uint64_t n) {
        uint64_t nn = n / block_size + (n % block_size ? 1 : 0);
        uint64_t leaves = nn;
        while (nn > block_size) {
            nn = nn / block_size + (nn % block_size ? 1 : 0);
            levels_++;
        }
        uint64_t n_lev = 1;
        uint64_t t_nodes = n_lev;
        for (uint64_t i = 1; i < levels_; i++) {
            n_lev *= block_size;
            t_nodes += n_lev;
        }
        node_count_ = t_nodes + leaves;
        nodes_ = (node*)malloc((node_count_) * sizeof(node));
        std::fill_n(nodes_, node_count_, node());
        std::memcpy(reinterpret_cast<uint64_t*>(nodes_ + t_nodes), data, n * sizeof(uint64_t));
        for (uint64_t i = t_nodes - 1; i < t_nodes; i--) {
            for (uint64_t k = 0; k < block_size; k++) {
                uint64_t c_idx = i * block_size + k + 1;
                if (c_idx < node_count_) {
                    nodes_[i].children[k] =
                        nodes_[i * block_size + k + 1].children[0];
                }
            }
        }
    }

    std::pair<uint64_t, uint64_t> find(uint64_t q) const {
        std::pair<uint64_t, uint64_t> ret = {0, 0};
        uint64_t n_idx = 0;
        for (uint64_t i = 0; i <= levels_; i++) {
            auto res = nodes_[n_idx].find(q);
            ret = {res.first, ret.second * block_size + res.second};
            n_idx = n_idx * block_size + 1 + res.second;
        }
        return ret;
    }

    void print() {
        std::cout << "B-heap search tree with " << node_count_ << " blocks"
                  << std::endl;
        for (uint64_t i = 0; i < node_count_; i++) {
            std::cout << " node " << i << std::endl;
            nodes_[i].print();
        }
    }

    uint64_t bytes() {
        return sizeof(b_heap_search) + node_count_ * sizeof(node);
    }
};
