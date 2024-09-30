#include "ds_construction.hpp"

void bottom_up_limit(const std::vector<std::vector<std::uint32_t>>& color_sets,
                    std::vector<std::int64_t>& parent_vec,
                    const std::int64_t depth_limit) {
    std::vector<std::int64_t> depth_vec(color_sets.size(), -1);

    for (std::int64_t i = 0; i < color_sets.size(); ++i) {
        if (depth_vec[i] == -1) {
            std::vector<std::int64_t> st;
            st.push_back(i);

            std::int64_t parent = parent_vec[i];
            while (parent != -1) {
                st.push_back(parent);
                parent = parent_vec[parent];
            }

            const std::int64_t root = st.back();
            std::int64_t depth = 0;

            while (st.size()) {
                if (depth > depth_limit) {
                    parent_vec[st.back()] = -1;
                    depth = 0;
                }

                const auto top = st.back(); st.pop_back();
                depth_vec[top] = depth++;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::fprintf(stderr, "usage: %s [color sets file] [parents file] [depth limit] [output file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    const auto color_sets = get_color_sets<std::uint32_t>(argv[1]);
    auto parents = get_parents<std::int64_t>(argv[2]);
    const std::int32_t depth_limit = std::stoi(argv[3]);

    std::cout << "Computing depths\n";
    bottom_up_limit(color_sets, parents, depth_limit);

    std::int64_t enc_width = 0;

    for (const auto& cs : color_sets) {
        for (const auto x : cs) {
            const std::int64_t bits = bits_required(x);
            enc_width = std::max(enc_width, bits);
        }
    }

    std::cout << "depth limit: " << depth_limit << "\n";
    std::cout << "encoding width: " << enc_width << "\n";

    const auto [d, m] = build_ds(color_sets, parents, enc_width);

    std::cout << "d.dense_container.size() "  << d.dense_container.size()  << "\n";
    std::cout << "d.dense_starts.size() "     << d.dense_starts.size()     << "\n";
    std::cout << "d.sparse_container.size() " << d.sparse_container.size() << "\n";
    std::cout << "d.sparse_starts.size() "    << d.sparse_starts.size()    << "\n";
    std::cout << "d.subset_container.size() " << d.subset_container.size() << "\n";
    std::cout << "d.subset_starts.size() "    << d.subset_starts.size()    << "\n";
    std::cout << "d.ancestor_ptrs.size() "    << d.ancestor_ptrs.size()    << "\n";
    std::cout << "\n";
    std::cout << "size in bytes: " << d.size_in_bytes() << "\n";

    std::ofstream ofs(argv[4]);
    const auto bw = d.serialize(ofs);
    std::cout << "bytes written: " << bw << "\n";
    ofs.close();
}
