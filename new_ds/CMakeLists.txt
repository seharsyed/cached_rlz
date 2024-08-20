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

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)
  target_link_libraries(benchmark PUBLIC OpenMP::OpenMP_CXX)
endif()
