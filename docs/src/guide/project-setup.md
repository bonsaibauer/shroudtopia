# 1. Create a Project

## Requirements

| Tool | Minimum |
|---|---|
| CMake | 3.24 |
| Compiler | Visual Studio 2022 x64 C++ toolchain |
| C++ standard | C++20 |

```powershell
cmake -S . -B build -A x64 -DSHROUDTOPIA_ROOT=A:/Github/shroudtopia
cmake --build build --config Release
```

```cmake
cmake_minimum_required(VERSION 3.24)
project(my_first_mod LANGUAGES CXX)
set(SHROUDTOPIA_ROOT "" CACHE PATH "Path to Shroudtopia")
if(NOT EXISTS "${SHROUDTOPIA_ROOT}/api/include/shroudtopia/api.h")
  message(FATAL_ERROR "Set SHROUDTOPIA_ROOT to a Shroudtopia checkout")
endif()
add_library(my-first-mod SHARED src/mod.cpp)
target_compile_features(my-first-mod PRIVATE cxx_std_20)
target_include_directories(my-first-mod PRIVATE "${SHROUDTOPIA_ROOT}/api/include")
set_target_properties(my-first-mod PROPERTIES PREFIX "")
```

The mod does not link against loader internals. `shroudtopia/api.h` is the public
entry point.
