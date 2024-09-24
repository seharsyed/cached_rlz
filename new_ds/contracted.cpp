#include "ds_construction.hpp"

std::vector<std::int64_t> contract_parents(const std::vector<std::vector<std::uint32_t>>& color_sets,
                                           const std::vector<std::int64_t>& parent_vec) {
    std::cout << "Computing contractions\n";

    std::vector<std::int64_t> cpar_vec(color_sets.size(), -1);

    for (std::int64_t i = 0; i < color_sets.size(); ++i) {
        if (cpar_vec[i] == -1 && parent_vec[i] != -1) {
            std::vector<std::int64_t> st;
            st.push_back(i);

            std::int64_t parent = parent_vec[i];
            while (parent != -1) {
                st.push_back(parent);
                parent = parent_vec[parent];
            }

            const std::int64_t root = st.back();
            cpar_vec[root] = -1;

            for (std::int64_t i = 0; i < st.size() - 1; ++i) {
                const auto s = st[i];
                cpar_vec[s] = parent_vec[s];

                for (std::int64_t j = i + 1; j < st.size(); ++j) {
                    const auto cpar = st[j];
                    if (color_sets[cpar].size() <= 2 * color_sets[s].size()) {
                        cpar_vec[s] = cpar;
                    }
                }
            }
        }
    }

    return cpar_vec;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::fprintf(stderr, "usage: %s [color sets file] [parents file] [output file]\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    std::cout << "Reading color sets\n";
    const auto color_sets = get_color_sets<std::uint32_t>(argv[1]);
    std::cout << "Reading parents\n";
    const auto parents = get_parents<std::int64_t>(argv[2]);
    std::cout << "Computing contracted parents\n";
    auto contracted_parents = contract_parents(color_sets, parents);

    std::cout << "Computing encoding width\n";

    std::int64_t enc_width = 0;

    for (const auto& cs : color_sets) {
        for (const auto x : cs) {
            const std::int64_t bits = bits_required(x);
            enc_width = std::max(enc_width, bits);
        }
    }

    std::cout << "encoding width: " << enc_width << "\n";

    const auto [d, m] = build_ds(color_sets, contracted_parents, enc_width);

    std::cout << "d.dense_container.size() "  << d.dense_container.size()  << "\n";
    std::cout << "d.dense_starts.size() "     << d.dense_starts.size()     << "\n";
    std::cout << "d.sparse_container.size() " << d.sparse_container.size() << "\n";
    std::cout << "d.sparse_starts.size() "    << d.sparse_starts.size()    << "\n";
    std::cout << "d.subset_container.size() " << d.subset_container.size() << "\n";
    std::cout << "d.subset_starts.size() "    << d.subset_starts.size()    << "\n";
    std::cout << "d.ancestor_ptrs.size() "    << d.ancestor_ptrs.size()    << "\n";
    std::cout << "\n";
    std::cout << "size in bytes: " << d.size_in_bytes() << "\n";

    std::ofstream ofs(argv[3]);
    const auto bw = d.serialize(ofs);
    std::cout << "bytes written: " << bw << "\n";
    ofs.close();
}
