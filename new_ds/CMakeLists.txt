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

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)
  target_link_libraries(benchmark PUBLIC OpenMP::OpenMP_CXX)
  target_link_libraries(top_down PUBLIC OpenMP::OpenMP_CXX)
  target_link_libraries(bottom_up PUBLIC OpenMP::OpenMP_CXX)
  target_link_libraries(contracted PUBLIC OpenMP::OpenMP_CXX)
  target_link_libraries(find_parents PUBLIC OpenMP::OpenMP_CXX)
endif()
