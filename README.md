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

# Note:

- Lexer is stateful (context-sensitive), because of the nature of XML.

- How to handle spaces in text??

- Only ASCII, UTF-8 is not supported.

- TODO: lexer data handling needs to be refactored

- Use "std::expected" instead of exceptions?
  -> probably not necessary, because we expect the input to be valid and if it is not, the program will exit anyway

## Known bugs

- column numbers are not correct if the file contains non-ASCII characters
- TASKS
