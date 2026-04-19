# Rotating Triangle

A simple Vulkan example that renders a rotating, color-cycling triangle using the generated `vk::` C++ wrapper (`out.hpp` / `out.cpp`).

> **Note:** This example is subject to change as the generator evolves.

## Prerequisites

1. **vkg_gen** — the generator must be compiled first and used to produce `out.cpp` and `out.hpp` (by default into `build/`).
2. **Vulkan** — a working Vulkan driver and loader must be installed on the system.
3. **GLFW** — used for window creation and input (`libglfw3-dev` / `glfw-devel`).
4. **glslc** — the Vulkan GLSL shader compiler (ships with the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) or available as `shaderc`).
5. **g++/clang++** new enough with C++17 support.

## Setup

The example expects `out.cpp` and `out.hpp` to be available in this directory. The easiest way is to create symbolic links pointing to the generator build output:

```bash
ln -s ../../build/out.cpp out.cpp
ln -s ../../build/out.hpp out.hpp
```

These symlinks are not provided in the repository.

## Build & Run

```bash
make
./vkg_triangle
```

`make` will:
- Compile the GLSL shaders (`shader_rotating.vert`, `shader.frag`) into SPIR-V using `glslc`.
- Compile `vkg_triangle.cpp` together with `out.cpp` into the `vkg_triangle` executable.

## Platform

Tested on **Linux only**. Other platforms are not currently supported.
