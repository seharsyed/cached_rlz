# Prerequisites

C++20 compiler, CMake 3.15 or newer

# Compiling

```
cmake -S . -B build -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc)
cmake --build build
```

# Running

This program generates a reference string from an input by random
sampling.

The function `generate_dictionary<T>(…)` generates a reference string
of type `T` for an input of type `T`.

```
./build/refgen [input file] [number of samples] [sample length] [output file] [seed]
```
