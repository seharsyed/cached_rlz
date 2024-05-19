cmake_minimum_required(VERSION 3.15)
project(color_set_refgen)

add_executable(refgen main.cpp generate_dictionary.hpp)
target_compile_features(refgen PUBLIC cxx_std_20)
target_compile_options(refgen PUBLIC -O3)

add_executable(ssgen generate_subset.cpp generate_dictionary.hpp)
target_compile_features(ssgen PUBLIC cxx_std_20)
target_compile_options(ssgen PUBLIC -O3)

add_executable(sscount count_subsets.cpp generate_dictionary.hpp)
target_compile_features(sscount PUBLIC cxx_std_20)
target_compile_options(sscount PUBLIC -O3)

add_executable(sstrcount count_substrings.cpp generate_dictionary.hpp)
target_compile_features(sstrcount PUBLIC cxx_std_20)
target_compile_options(sstrcount PUBLIC -O3)

add_executable(exss extract_supersets.cpp generate_dictionary.hpp)
target_compile_features(exss PUBLIC cxx_std_20)
target_compile_options(exss PUBLIC -O3)
