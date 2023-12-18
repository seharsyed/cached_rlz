# Prerequisites

C++20 compiler, CMake 3.15 or newer

# Compiling

```
cmake -S . -B build -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc)
cmake --build build
```

# Running

This program builds a suffix array out of input file.

The function `prefix_doubling<T1, T2>(…)` computes suffix array of
type `T1` for an input of type `T2`.

```
./build/sa [input file] [output file]
```
