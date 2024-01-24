cmake_minimum_required(VERSION 3.15)
project(parser)
include(ExternalProject)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/sdsl-lite
  ${PROJECT_BINARY_DIR}/external/sdsl-lite/build)

add_executable(parser main.cpp parser.hpp random_access_rlz.hpp)
target_compile_features(parser PUBLIC cxx_std_20)
target_link_libraries(parser PRIVATE sdsl)

add_executable(benchmark benchmark.cpp parser.hpp random_access_rlz.hpp)
target_compile_features(benchmark PUBLIC cxx_std_20)
target_compile_options(benchmark PUBLIC -O3)
target_link_libraries(benchmark PRIVATE sdsl)
