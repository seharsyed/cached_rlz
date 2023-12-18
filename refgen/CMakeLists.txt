cmake_minimum_required(VERSION 3.15)
project(refgen)

add_executable(refgen main.cpp generate_dictionary.hpp)
target_compile_features(refgen PUBLIC cxx_std_20)
