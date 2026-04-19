# Vulkan C++ API Generator

Vulkan C++ API Generator named **vkgen** is a configurable C++ Vulkan bindings generator that reads `vk.xml` (Khronos Vulkan registry) and produces a lean `.hpp`/`.cpp` pair tailored to the selected Vulkan version and extensions.

This project is developed as a bachelor's thesis at **Brno University of Technology, Faculty of Information Technology ([VUT FIT](https://www.fit.vutbr.cz))** under the supervision of **Ing. Jan Pečiva, Ph.D.** ([PCJohn](https://github.com/pc-john)), author of the reference `vkg.h` / `vkg.cpp` (found in [VulkanTutorial](https://github.com/Vulkan-FIT/VulkanTutorial))headers that define the output style targeted by this generator.

This project is part of the [Vulkan-FIT](https://github.com/Vulkan-FIT) project.

## Building

This project is WIP. Compiling tested on Fedora 43 with `GCC 15.2.1` and `Clang 21.1.8` and Windows 11 with `MSVC 19.44.35225`. Tested generators are `ninja`, `make` and `vs2022`.
Also tested on **Ubuntu 24.04** in Docker (for more details see /tests/compiling).

- [x] GCC 13 - 15
- [x] Clang 18 - 21
- [x] aarch64-linux-gnu
- [x] aarch64-clang++

To build the generator, run:

```bash
mkdir build && cd build
cmake .. -G Ninja # you can also use `make`, or `vs2022`
ninja
```

The default build type is **Debug**. To select a different profile:

```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release        # optimized, no debug info
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo  # optimized + debug info
```

To enable **Address, Leak, and Undefined Behavior sanitizers** (forces Debug):

```bash
cmake .. -G Ninja -DSANITIZE=ON
```

## Usage

```
./vkgen [options]
```

Run `./vkgen --help` for a full list of options. Key flags:

- `--config <path>` — load a config file (key=value format)
- `--create-config <path>` — generate a default config file
- `--xml <path>` — path to `vk.xml` (default: `vk.xml`)
- `--header <path>` / `--source <path>` — output file paths (default: `vkg.hpp` / `vkg.cpp`)
- `--version <1.X>` — target Vulkan version (e.g. `1.3`)
- `--ext <list>` — comma-separated extension list

CLI arguments override config file values. Config file overrides defaults.

## Examples

The `examples/` directory contains sample applications that use the generated wrapper:

- **[rotating_triangle](examples/rotating_triangle/)** — A Vulkan rotating triangle demo using the `vk::` API. See its README for build instructions and prerequisites.

### VulkanTutorial

Generated `vkg.h` / `vkg.cpp` headers are compatible (with minor changes) with the examples in [VulkanTutorial](https://github.com/Vulkan-FIT/VulkanTutorial).

# Limitations:

- Lexer is stateful (context-sensitive), because of the nature of XML.

- Only ASCII, UTF-8 is not supported.
- Only generating "vulkan" API not "vulkansc"

## Task List

### P0 — Must-Have

- [x] P0-1: Namespace wrapping (`namespace vk { }`, rename FuncTable → Funcs)
- [x] P0-2: Command name translation (`vkCreateBuffer` → `createBuffer`)
- [x] P0-3: Enum extension deduplication (TASK 030226_01)
- [x] P0-4: Command alias resolution (generate forwarding wrappers)
- [x] P0-5: Header boilerplate matching ref (UniqueHandle, detail namespace, error classes, init declarations)
- [x] P0-6: Throw/NoThrow command generation (void/VkResult patterns, handle translation, implicit device/instance)
- [x] P0-7: Source file (.cpp) generation + deferred command patterns
  - [x] P0-7a: Enumerate-style commands (two-call pattern, vector return, .cpp implementations)
  - [x] P0-7b: PhysicalDevice convenience overloads (default to `physicalDevice()`)
  - [x] P0-7c: UniqueHandle create variants (`createXxxUnique_throw/noThrow`)
  - [x] P0-7d: Proper `throwResultException` with per-Result exception mapping
  - [ ] P0-7e: `resultToString`, `Error` constructors
  - [x] P0-7f: `loadLib_throw/noThrow`, `initInstance`, `initDevice` with PFN loading
  - [x] P0-7g: Instance-level vs device-level PFN loading split
  - [x] P0-7h: Input array → `span<T>` conversion (1:1 count/array pairs, includes handles), also handles the case of single item "array"
- [x] P0-8: Config from file + CLI arguments

### P1 — Important Quality

- [ ] P1-1: Basetype dependencies (TASK 220326_01)
- [ ] P1-2: Empty bitmasks (TASK 230326_02)
- [ ] P1-3: Array enum dependencies (TASK 210326_02)
- [ ] P1-4: TypeParam word boundary fix
- [ ] P1-5: Dead code cleanup
- [ ] P1-6: MSVC compatibility of generated code
- [ ] P1-7: Shared-count input arrays (one count → multiple arrays)
  - [x] P1-7a: Shared-count input arrays for creating multiple handles
  - [ ] P1-7b: Other share-count input arrays
- [ ] P1-8: Complex `len` expressions (latexmath, commas)
- [x] P1-9: ~~Handle arrays in input array conversion~~ (resolved by `span<T>`)
- [ ] P1-10: Other original vulkan convinience functions
  - [x] P1-10a: `to_cstr(<enum>)`
  - [ ] P1-10b: TODO:

### P2 — After MVP

- [ ] P2-1: Test on VulkanTutorial example
- [ ] P2-2: Compilation speed benchmark vs vulkan-hpp
- [ ] P2-3: CMake integration as dependency
- [ ] P2-4: C++ module support research
- [ ] P2-5: Generator code restructuring

### P3 — Future / Optional

- [ ] P3-1: Refactor lexers data handling
- [ ] P3-2: Update parser to work more closely with generator
- [ ] P3-3: Better error handling and reporting
- [ ] P3-4: Proper testing pipeline
  - [ ] P3-4a: Test compiling with many compilers and configurations (also test for warnings)
  - [ ] P3-4b: Test running vkgen with different configs
  - [ ] P3-4c: Test compiling Release, RelWithDebInfo and Debug
  - [ ] P3-4d: Test running vkgen with memory, address, and undefined behavior sanitizers
  - [ ] P3-4e: Test the actual generated code (compiling, running, ...)
  - [ ] P3-4f: Add "blackbox" tests for vkgen

## Testing

The `tests/compiling/` directory contains scripts to verify the generator compiles and produces correct output across compilers and configurations.
It is supposed to be used as an reference to see, what was tested.

If you want to run tests on your local machine, you need also need to have `Docker` installed.
The reference output (vkg.cpp and vkg.hpp) and testing file (vkg_comp.cpp) are expected to be provided in the `tests/compiling/golden/` directory.

**Local tests** — builds and runs the generator in Debug, Release, and RelWithDebInfo, then diffs output against golden files:

```bash
cd tests/compiling
./run_local_tests.sh
```

**Cross-compiler matrix** (Docker) — tests GCC 13–15, Clang 18–21, and aarch64 cross-compilation inside an Ubuntu 24.04 container:

```bash
cd tests/compiling
./setup_matrix.sh   # build the Docker image (one-time)
./run_matrix.sh     # run the full matrix
```

## Known bugs

- column numbers are not correct if the file contains non-ASCII characters
- TASKS
