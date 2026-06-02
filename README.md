# Parser

Parser contains the original implementation of the LZ parser and the random-access RLZ index.

# Cached RLZ Parser

This repository also contains an experimental cached RLZ parser wrapper:

```text
parser/cached_rlz_parser.hpp
```

The cached parser wraps the RLZ factorisation process and adds a cache for repeated suffix-array interval refinements. When the same refinement is needed again, the parser can reuse the cached interval instead of repeating the binary searches.

The comparison program is:

```text
parser/compare_rlz_cache.cpp
```

This program runs both the original RLZ parser and the cached RLZ parser, checks that they produce the same factorisation output, and reports timing and cache statistics such as hits, misses, number of entries, and hit rate.

# Refgen

Refgen contains a program that generates a reference string of arbitrary integer type for an input of that type.

# SA

SA contains a simple implementation of the prefix-doubling algorithm to compute suffix arrays of arbitrary integer type for inputs of arbitrary integer types.

