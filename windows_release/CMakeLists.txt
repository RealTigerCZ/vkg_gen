cmake_minimum_required(VERSION 3.21)

set(APP_NAME vkgen)
set(CMAKE_CXX_STANDARD 23)

project(${APP_NAME})

set(APP_SOURCES
   ../src/arena.cpp
   ../src/config.cpp
   ../src/xml/xml.cpp
   ../src/xml/lexer.cpp
   ../src/xml/parser.cpp
   ../src/generator/generator.cpp
   ../src/main.cpp
   ./vkgen.rc  # Resource file
)

add_executable(${APP_NAME} ${APP_SOURCES})

target_compile_options(${APP_NAME} PRIVATE /W4 /Zc:preprocessor)

# Bakes the C++ runtime into the .exe so it runs on vanilla Windows.
set_property(TARGET ${APP_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")

# MSVC Profile handling via Generator Expressions
target_compile_options(${APP_NAME} PRIVATE
    $<$<CONFIG:Release>:/O2>
)
