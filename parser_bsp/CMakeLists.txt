cmake_minimum_required(VERSION 3.15)
project(parser_bs_vec)
include(ExternalProject)

add_executable(parser main.cpp parser.hpp random_access_rlz.hpp)
target_compile_features(parser PUBLIC cxx_std_20)

add_executable(benchmark benchmark.cpp parser.hpp random_access_rlz.hpp)
target_compile_features(benchmark PUBLIC cxx_std_20)
target_compile_options(benchmark PUBLIC -O3)
