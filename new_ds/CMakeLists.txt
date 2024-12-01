cmake_minimum_required(VERSION 3.15)
project(new_ds)
include(ExternalProject)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/sdsl-lite
  ${PROJECT_BINARY_DIR}/external/sdsl-lite/build)

add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_20)
target_compile_options(main PRIVATE -O3)
target_link_libraries(main PRIVATE sdsl)

add_executable(benchmark benchmark.cpp)
target_compile_features(benchmark PRIVATE cxx_std_20)
target_compile_options(benchmark PRIVATE -O3)
target_link_libraries(benchmark PRIVATE sdsl)

add_executable(top_down top_down.cpp)
target_compile_features(top_down PRIVATE cxx_std_20)
target_compile_options(top_down PRIVATE -O3)
target_link_libraries(top_down PRIVATE sdsl)

add_executable(bottom_up bottom_up.cpp)
target_compile_features(bottom_up PRIVATE cxx_std_20)
target_compile_options(bottom_up PRIVATE -O3)
target_link_libraries(bottom_up PRIVATE sdsl)

add_executable(contracted contracted.cpp)
target_compile_features(contracted PRIVATE cxx_std_20)
target_compile_options(contracted PRIVATE -O3)
target_link_libraries(contracted PRIVATE sdsl)

add_executable(find_parents find_parents.cpp)
target_compile_features(find_parents PRIVATE cxx_std_20)
target_compile_options(find_parents PRIVATE -O3)
target_link_libraries(find_parents PRIVATE sdsl)

add_executable(top_down_lno top_down_lno.cpp)
target_compile_features(top_down_lno PRIVATE cxx_std_20)
target_compile_options(top_down_lno PRIVATE -O3)
target_link_libraries(top_down_lno PRIVATE sdsl)

add_executable(stats stats.cpp)
target_compile_features(stats PRIVATE cxx_std_20)
target_compile_options(stats PRIVATE -O3)
target_link_libraries(stats PRIVATE sdsl)

add_executable(sd_benchmark sd_benchmark.cpp)
target_compile_features(sd_benchmark PRIVATE cxx_std_20)
target_compile_options(sd_benchmark PRIVATE -O3)
target_link_libraries(sd_benchmark PRIVATE sdsl)

add_executable(parent_stats parent_stats.cpp)
target_compile_features(parent_stats PRIVATE cxx_std_20)
target_compile_options(parent_stats PRIVATE -O3)
target_link_libraries(parent_stats PRIVATE sdsl)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)
  target_link_libraries(find_parents PUBLIC OpenMP::OpenMP_CXX)
endif()
