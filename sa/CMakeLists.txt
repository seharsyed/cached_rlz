cmake_minimum_required(VERSION 3.15)
project(sa)

add_executable(sa suffix_array.cpp)
target_compile_features(sa PRIVATE cxx_std_20)
target_compile_options(sa PRIVATE -O3)

add_executable(psa parallel_suffix_array.cpp)
target_compile_features(psa PRIVATE cxx_std_20)
target_compile_options(psa PRIVATE -O3)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(psa PUBLIC OpenMP::OpenMP_CXX)
endif()
