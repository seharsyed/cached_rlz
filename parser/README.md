# Prerequisites

C++20 compiler, CMake 3.15 or newer, Git

# Compiling

```
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc)
cmake --build build
```

# Running

This program does RLZ parsing of input from reference file. Does not
output anything if uncompressed data can be recreated from compressed
data. Optionally outputs `(source position, reference position,
length)`-triplets if output file is supplied.

Mismatches or matches of length 1 have the symbol embedded in the
reference pointer.

The function `lzFactorize<T1, T2>(…)` takes the type of data as
template parameter `T1` and the type of suffix array as template
parameter `T2`.

The data structure `random_access_rlz<T>` is an RLZ index that
supports random access. It takes the type of data as template parameter `T`.

```
./build/parser [reference file] [suffix array file] [input file] {output file}
```
