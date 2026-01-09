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

# TODO: debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -pedantic -O0 -fdiagnostics-color=always")
# TODO: remove this
target_compile_options(${APP_NAME} PRIVATE -fdiagnostics-color=always)
# Assuming your target is named vkg_gen
target_link_libraries(vkg_gen
    PRIVATE
    stdc++exp           # <stacktrace>
)

# Debug target to check if out.hpp is compilable
add_custom_target(out_comp
    COMMAND g++ -c ${CMAKE_BINARY_DIR}/out.hpp -o ${CMAKE_BINARY_DIR}/vk.o -std=c++20 -Wall -Wextra -g -pedantic -fdiagnostics-color=always
    COMMAND echo "" && echo "Successfully compiled out.hpp!"
    COMMENT "Compiling out.hpp manually with g++ to check if it compiles"
)
