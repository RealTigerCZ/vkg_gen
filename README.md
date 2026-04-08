# Vulkan C++ API Generator

## Building

This project is WIP. Tested on Fedora 43 with `GCC 15.2.1` and `Clang 21.1.8`. Tested generators are `ninja` and `make`. Does NOT work with MSVC yet.

```bash
mkdir build && cd build
cmake ..
cmake --build . # or you can use `ninja` / `make` directly
```

Then don't forget to copy the `vk.xml` into build directory and run the generator:

```bash
cp ../../vk.xml . # copy vk.xml to build directory
./vkg_gen
```

To produce the `out.cpp` and `out.hpp` files.

## Examples

The `examples/` directory contains sample applications that use the generated wrapper:

- **[rotating_triangle](examples/rotating_triangle/)** — A Vulkan rotating triangle demo using the `vk::` API. See its README for build instructions and prerequisites.

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
  - [x] P0-7e: `resultToString`, `Error` constructors
  - [x] P0-7f: `loadLib_throw/noThrow`, `initInstance`, `initDevice` with PFN loading
  - [x] P0-7g: Instance-level vs device-level PFN loading split
- [ ] P0-8: Config from file + CLI arguments

### P1 — Important Quality

- [ ] P1-1: Basetype dependencies (TASK 220326_01)
- [ ] P1-2: Empty bitmasks (TASK 230326_02)
- [ ] P1-3: Array enum dependencies (TASK 210326_02)
- [ ] P1-4: TypeParam word boundary fix
- [ ] P1-5: Dead code cleanup
- [ ] P1-6: MSVC compatibility of generated code

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

## Known bugs

- column numbers are not correct if the file contains non-ASCII characters
- TASKS
