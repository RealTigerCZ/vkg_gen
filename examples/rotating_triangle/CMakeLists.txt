cmake_minimum_required(VERSION 3.21)
project(vkg_triangle CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Pull in vkgen + the vkg_generate() / vkg_fetch_vk_xml() helpers.
add_subdirectory(../.. vkg_build)

find_package(Vulkan REQUIRED)

find_package(glfw3 CONFIG QUIET)
if(NOT glfw3_FOUND)
    include(FetchContent)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    FetchContent_MakeAvailable(glfw)
endif()

# Pin vk.xml to the revision this example was last tested against.
set(VKG_DEFAULT_VK_XML_URL
    "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/72cd1f587fe55c80873fe6430d667056048a5113/xml/vk.xml"
    CACHE STRING "" FORCE)
vkg_fetch_vk_xml(VKG_TRIANGLE_XML)

vkg_generate(vkg_triangle_lib
    XML    ${VKG_TRIANGLE_XML}
    CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/triangle.cfg
)

add_executable(vkg_triangle vkg_triangle.cpp)
target_link_libraries(vkg_triangle PRIVATE vkg_triangle_lib Vulkan::Headers glfw)

if(MSVC)
    target_compile_options(vkg_triangle PRIVATE /W3 /wd4996)
else()
    target_compile_options(vkg_triangle PRIVATE -Wno-deprecated-declarations)
endif()

find_program(GLSLC glslc HINTS ENV VULKAN_SDK PATH_SUFFIXES Bin bin REQUIRED)

function(compile_shader SRC OUT)
    add_custom_command(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${OUT}
        COMMAND ${GLSLC} ${CMAKE_CURRENT_SOURCE_DIR}/${SRC} -o ${CMAKE_CURRENT_BINARY_DIR}/${OUT}
        DEPENDS ${SRC}
        VERBATIM
    )
endfunction()

compile_shader(shader_rotating.vert vert_rotating.spv)
compile_shader(shader.frag          frag.spv)

add_custom_target(shaders ALL DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/vert_rotating.spv
    ${CMAKE_CURRENT_BINARY_DIR}/frag.spv
)
add_dependencies(vkg_triangle shaders)
