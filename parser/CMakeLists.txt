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

add_executable(color_set_parser color_set_parser.cpp color_set_parser.hpp parser.hpp random_access_rlz.hpp)
target_compile_features(color_set_parser PUBLIC cxx_std_20)
target_compile_options(color_set_parser PUBLIC -O3)
target_link_libraries(color_set_parser PRIVATE sdsl)

add_executable(read_parsing read_parsing.cpp color_set_parser.hpp parser.hpp random_access_rlz.hpp)
target_compile_features(read_parsing PUBLIC cxx_std_20)
target_compile_options(read_parsing PUBLIC -O3)
target_link_libraries(read_parsing PRIVATE sdsl)

add_executable(write_parsing write_parsing.cpp color_set_parser.hpp parser.hpp random_access_rlz.hpp)
target_compile_features(write_parsing PUBLIC cxx_std_20)
target_compile_options(write_parsing PUBLIC -O3)
target_link_libraries(write_parsing PRIVATE sdsl)
