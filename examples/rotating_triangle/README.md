# Rotating Triangle

A simple Vulkan example that renders a rotating, color-cycling triangle using the generated `vk::` C++ wrapper (`vkg.hpp` / `vkg.cpp`).

> **Note:** This example is subject to change as the generator evolves.

## Prerequisites

1. **vkgen** — the generator must be compiled first and used to produce `vkg.cpp` and `vkg.hpp` (by default into `build/`).
2. **Vulkan** — a working Vulkan driver and loader. Compiling also needs the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) installed (provides headers and `glslc`).
3. **GLFW** — used for window creation and input. On Linux it's found via the system package (`libglfw3-dev` / `glfw-devel`). On Windows CMake downloads and builds GLFW automatically on the first configure - easier to use.
4. **CMake ≥ 3.20** and a C++20 compiler (g++/clang++ on Linux, MSVC on Windows).

## Setup

The example expects `vkg.cpp` and `vkg.hpp` in this directory. Generate them first:

```bash
./vkgen --config triangle.cfg --xml <path/to/vk.xml>
```

## Build & Run

Linux:

```bash
cmake -B build
cmake --build build
cd build && ./vkg_triangle
```

Windows (MSVC):

```bat
cmake -B build
cmake --build build --config Release
cd build\Release && vkg_triangle.exe
```

If you have GLFW installed via vcpkg (`vcpkg install glfw3:x64-windows`), pass the toolchain file at configure time to use it instead of downloading a fresh copy:

```bat
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
```

The build will:

- Compile the GLSL shaders (`shader_rotating.vert`, `shader.frag`) into SPIR-V via `glslc`.
- Compile `vkg_triangle.cpp` together with `vkg.cpp` into the `vkg_triangle` executable.

The binary loads `vert_rotating.spv` and `frag.spv` from the current working directory, so run it from the build directory where those files are produced.

## Platform

Builds on Linux (g++/clang++) and Windows (MSVC).
