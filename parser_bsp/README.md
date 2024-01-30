# Prerequisites

C++20 compiler, CMake 3.15 or newer, Git

# Compiling

```
cmake -S . -B build -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc)
cmake --build build
```

# Running

This program implements random access RLZ-index by storing starts in a
std::vector and uses std::lower_bound for predecessor queries to
implement access.

```
./build/parser [reference file] [suffix array file] [input file] {output file}
```
