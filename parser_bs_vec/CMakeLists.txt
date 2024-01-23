cmake_minimum_required(VERSION 3.15)
project(parser_bs_vec)
include(ExternalProject)

add_executable(parser main.cpp parser.hpp random_access_rlz.hpp)
target_compile_features(parser PUBLIC cxx_std_20)
