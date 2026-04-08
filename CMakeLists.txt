# TODO: this cmake should be inlinable into the build target for other projects

cmake_minimum_required(VERSION 3.15) # CHECK: try other versions later

set(APP_NAME vkg_gen)
set(CMAKE_CXX_STANDARD 23)

project(${APP_NAME})

set(APP_SOURCES
   src/arena.cpp
   src/xml/xml.cpp
   src/xml/lexer.cpp
   src/xml/parser.cpp
   src/generator/generator.cpp
   src/main.cpp
   )

set(APP_INCLUDES

   )

# target
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES})

if(MSVC)
    # MSVC (Windows) specific flags
    # /W4: High warning level
    # /Zi: Generate debug info (-g)
    # /Od: Disable optimizations (-O0)
    target_compile_options(${APP_NAME} PRIVATE /W4 /Zi /Od)

    # MSVC might require /Zc:preprocessor for modern C++ compliance
    target_compile_options(${APP_NAME} PRIVATE /Zc:preprocessor)
else()
    target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -g -pedantic -O0 -fdiagnostics-color=always)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${APP_NAME} PRIVATE stdc++exp) # <stacktrace>
    endif()
endif()

# Debug target to check if out.hpp is compilable
if (MSVC)
    add_custom_target(out_comp COMMAND echo "Compiling out.hpp manually with MSVC is not supported")
else()
    add_custom_target(out_comp
        COMMAND ${CMAKE_CXX_COMPILER} -c ${CMAKE_BINARY_DIR}/out.cpp -o ${CMAKE_BINARY_DIR}/vk.o -std=c++20 -Wall -Wextra -g -pedantic -fdiagnostics-color=always
        COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_BINARY_DIR}/out_comp.cpp -g ${CMAKE_BINARY_DIR}/vk.o -o ${CMAKE_BINARY_DIR}/out
        COMMAND echo "" && echo "Successfully compiled test file 'out_comp.cpp'!"
        COMMENT "Compiling out.hpp manually with gcc/clang to check if it compiles"
    )
endif()
