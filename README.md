# Vulkan C++ API Generator

# Note:

- Lexer is stateful (context-sensitive), because of the nature of XML.

- How to handle spaces in text??

- Only ASCII, UTF-8 is not supported.

- TODO: lexer data handling needs to be refactored

- Use "std::expected" instead of exceptions?
  -> probably not necessary, because we expect the input to be valid and if it is not, the program will exit anyway

## Known bugs

- column numbers are not correct if the file contains non-ASCII characters
