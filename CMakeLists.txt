# TODO: this cmake should be inlinable into the build target for other projects

cmake_minimum_required(VERSION 3.15) # CHECK: try other versions later

set(APP_NAME vkg_gen)
set(CMAKE_CXX_STANDARD 23)

project(${APP_NAME})

# Default to Debug build if no profile is specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build (Debug, RelWithDebugInfo, Release)" FORCE)
endif()

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

option(SANITIZE "Enable Address and Undefined Behavior Sanitizers" OFF)

# target
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES})

if(MSVC)
    # MSVC (Windows) specific base flags
    target_compile_options(${APP_NAME} PRIVATE /W4 /Zc:preprocessor)

    # MSVC Profile handling via Generator Expressions
    target_compile_options(${APP_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:RelWithDebugInfo>:/Zi /O2>
        $<$<CONFIG:Release>:/O2>
    )
else()
    # Always applied to GCC/Clang
    target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra)

    if (SANITIZE)
        # SANITIZE overrides standard profiles to force Debug + Sanitizer flags
        target_compile_options(${APP_NAME} PRIVATE
            -O0 -g -fdiagnostics-color=always -pedantic
            -fsanitize=address -fsanitize=leak -fsanitize=undefined
        )
        target_link_options(${APP_NAME} PRIVATE
            -fsanitize=address -fsanitize=leak -fsanitize=undefined
            -static-libasan -static-libubsan
        )
        message(STATUS "Build Profile: SANITIZE (Forced Debug + Sanitizers)")
    else()
        # Map requested profiles using Generator Expressions
        target_compile_options(${APP_NAME} PRIVATE
            $<$<CONFIG:Debug>:-O0 -g -fdiagnostics-color=always -pedantic>
            $<$<CONFIG:RelWithDebugInfo>:-O2 -g -fdiagnostics-color=always -pedantic>
            $<$<CONFIG:Release>:-O3>
        )
        message(STATUS "Build Profile: ${CMAKE_BUILD_TYPE}")
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${APP_NAME} PRIVATE stdc++exp) # <stacktrace>
    endif()
endif()

# Debug target to check if out.hpp is compilable
if (MSVC)
    add_custom_target(out_comp COMMAND echo "Compiling out.hpp manually with MSVC is not supported")
else()
    # This works for both GCC (GNU) and Clang
    add_custom_target(out_comp
        COMMAND ${CMAKE_CXX_COMPILER} -c ${CMAKE_BINARY_DIR}/out.cpp -o ${CMAKE_BINARY_DIR}/vk.o -std=c++20 -Wall -Wextra -g -pedantic -fdiagnostics-color=always
        COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_BINARY_DIR}/out_comp.cpp -g ${CMAKE_BINARY_DIR}/vk.o -o ${CMAKE_BINARY_DIR}/out
        COMMAND echo "" && echo "Successfully compiled test file 'out_comp.cpp' with ${CMAKE_CXX_COMPILER_ID}!"
        COMMENT "Compiling out.hpp manually with ${CMAKE_CXX_COMPILER_ID} to check if it compiles"
    )
endif()
