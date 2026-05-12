# Vulkan C++ API Generator

The Vulkan C++ API Generator named **vkgen** is a configurable C++ Vulkan bindings generator that reads `vk.xml` (Khronos Vulkan registry) and produces a lean `.hpp`/`.cpp` pair tailored to the selected Vulkan version and extensions.

This project is developed as a bachelor's thesis at **Brno University of Technology, Faculty of Information Technology ([VUT FIT](https://www.fit.vutbr.cz))** under the supervision of **Ing. Jan Pečiva, Ph.D.** ([PCJohn](https://github.com/pc-john)), author of the reference `vkg.h` / `vkg.cpp` (found in [VulkanTutorial](https://github.com/Vulkan-FIT/VulkanTutorial)) headers that define the output style targeted by this generator.

This project is part of the [Vulkan-FIT](https://github.com/Vulkan-FIT) project.

## Building

This project is WIP. Compiling tested on Fedora 43 with `GCC 15.2.1` and `Clang 21.1.8`, and Windows 11 with `MSVC 19.44.35225`. Tested generators are `ninja`, `make` and `vs2022`.
Also tested on **Ubuntu 24.04** in Docker (for more details, see /tests/compiling).

- [x] GCC 13 - 15
- [x] Clang 18 - 21
- [x] aarch64-linux-gnu
- [x] aarch64-clang++

### Including the project in other projects:
To include this generator as a part of another project, create a CMake file like this:
```cmake
cmake_minimum_required(VERSION 3.21)
project(vkg_cmake_sample CXX)

# Pull in the generator, and specify the output directory
add_subdirectory(<path/to/src/of/vkgen> vkg_build)


# If you don't want to provide your own vk.xml, use vkg_fetch_vk_xml()
# This may introduce problems if vk.xml is too new. So you can also provide your own URL like this:
SET (VKG_DEFAULT_VK_XML_URL "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/72cd1f587fe55c80873fe6430d667056048a5113/xml/vk.xml")
vkg_fetch_vk_xml(VKG_XML_PATH)

# Generate a library target "vkg_lib" from vk.xml + sample.cfg.
# The generator is invoked automatically before this target is built.
# Note: if your config renames the generated headers, you also need to pass the new names here.
vkg_generate(vkg_lib
    XML    ${VKG_XML_PATH}
    CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/sample.cfg # provide your own config
)

add_executable(sample main.cpp)
target_link_libraries(sample PRIVATE vkg_lib)
```
and compile it with your project. For more information, see the working example in `examples/cmake_integration`.


### Building only vkgen 
To build the generator, run:

```bash
cmake -B build
cmake --build build --config Release -j 
```
Available modes are: `Debug`, `RelWithDebInfo`, `Release`.

To enable **Address, Leak, and Undefined Behaviour sanitisers** (forces Debug):

```bash
cmake -B build -DSANITIZE=ON
cmake --build build -j
```

The produced executable will be located in `build/vkgen` on Linux or `build\<Profile>\vkgen.exe` on Windows.

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
- **[cmake_integration](examples/cmake_integration/)** — Simple example how to add generated `vkg` API as an target to your CMake project.

### VulkanTutorial

Generated `vkg.h` / `vkg.cpp` headers are compatible (with minor changes) with the examples in [VulkanTutorial](https://github.com/Vulkan-FIT/VulkanTutorial).
For the working demo, see the [Bachelor's thesis files](#bachelors-thesis).

## Bachelor's Thesis
As mentioned before, this project is part of a bachelor's thesis. All submitted work can be found [here](https://nextcloud.fit.vutbr.cz/s/88E4DjxigiEgneJ). This archive contains a demonstration of the VulkanTutorial example, speed measurements, the LaTeX source of the work, a PDF, and the source code of this repo. 
The thesis site itself can be found [here](https://www.vut.cz/en/students/final-thesis/detail/169667).


## Limitations:

- Lexer is stateful (context-sensitive), because of the nature of XML.

- Only ASCII, UTF-8 is not supported.
- Only generating "vulkan" API, not "vulkansc"

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
- [x] P1-6: MSVC compatibility of generated code
- [ ] P1-7: Shared-count input arrays (one count → multiple arrays)
  - [x] P1-7a: Shared-count input arrays for creating multiple handles
  - [ ] P1-7b: Other share-count input arrays
- [ ] P1-8: Complex `len` expressions (latexmath, commas)
- [x] P1-9: ~~Handle arrays in input array conversion~~ (resolved by `span<T>`)
- [ ] P1-10: Other original **vkg** convenience functions
  - [x] P1-10a: `to_cstr(<enum>)`
  - [ ] P1-10b: TODO:

### P2 — After MVP

- [x] P2-1: Test on VulkanTutorial example (see [Bacheleor's thesis files](#bacheleors-thesis))
- [x] P2-2: Compilation speed benchmark vs vulkan-hpp ([Bacheleor's thesis pdf](#bacheleors-thesis))
- [x] P2-3: CMake integration as dependency
- [x] P2-4: C++ module support research
- [ ] P2-5: Generator code restructuring

### P3 — Future / Optional

- [ ] P3-1: Refactor lexers data handling
- [ ] P3-2: Update parser to work more closely with generator
- [ ] P3-3: Better error handling and reporting
- [ ] P3-4: Proper testing pipeline
  - [ ] P3-4a: Test compiling with many compilers and configurations (also test for warnings)
  - [ ] P3-4b: Test running vkgen with different configs
  - [ ] P3-4c: Test compiling Release, RelWithDebInfo and Debug
  - [ ] P3-4d: Test running vkgen with memory, address, and undefined behaviour sanitisers
  - [ ] P3-4e: Test the actual generated code (compiling, running, ...)
  - [ ] P3-4f: Add "blackbox" tests for vkgen

## Testing

The `tests/compiling/` directory contains scripts to verify that the generator compiles and produces correct output across compilers and configurations.
It is supposed to be used as a reference to see what was tested.

If you want to run tests on your local machine, you also need to have `Docker` installed.
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

- Column numbers are not correct if the file contains non-ASCII characters
- TASKS

## AI usage

Parts of this project were developed with the assistance of AI tools, namely [Claude Code](https://claude.com/claude-code) (Anthropic), [Codeium](https://codeium.com/) and [Gemini](https://github.com/google/gemini) (Google). All AI-generated contributions were reviewed by the author except parts explicitly marked as "AI-generated".
