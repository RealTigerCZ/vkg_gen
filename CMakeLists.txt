# TODO: this cmake should be inlinable into the build target for other projects

cmake_minimum_required(VERSION 3.15) # CHECK: try other versions later 

set(APP_NAME vkg_gen)

project(${APP_NAME})

set(APP_SOURCES
    src/main.cpp
   )

set(APP_INCLUDES

   )

# target
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES})
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)
