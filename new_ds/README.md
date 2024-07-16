# Prerequisites

C++20 compiler, CMake 3.15 or newer, Git

# Compiling

```
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc)
cmake --build build
```
