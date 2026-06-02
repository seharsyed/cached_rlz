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

# Cached RLZ Parser

The cached RLZ parser is cached version of the RLZ parser.

The implementation is in:

```text
cached_rlz_parser.hpp
```

The cached version keeps the original parser unchanged. It adds a cache for repeated suffix-array interval refinement steps used during RLZ parsing.

The cache stores the result of refining a suffix-array interval by the next input symbol. If the same refinement is needed again, the cached interval is reused instead of performing the binary searches again.

The comparison program is:

```text
compare_rlz_cache.cpp
```

It runs both the original RLZ parser and the cached RLZ parser on the same input. It checks that both versions produce the same factors and reports timing and cache statistics.

```
./compare_rlz_cache [reference file] [suffix array file] [input file]
```

