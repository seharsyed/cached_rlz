cmake_minimum_required(VERSION 3.15)
project(sa)

add_executable(sa suffix_array.cpp)
target_compile_features(sa PUBLIC cxx_std_20)
