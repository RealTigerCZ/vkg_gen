cmake_minimum_required(VERSION 3.21)
project(vkg_cmake_sample CXX)

# Pull in the generator, and specify the output directory
add_subdirectory(../.. vkg_build)


# If you dont want to provide own vk.xml, use vkg_fetch_vk_xml()
# This may introduce problems, if vk.xml is too new. So you can also provide your own URL like this:
SET (VKG_DEFAULT_VK_XML_URL "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/72cd1f587fe55c80873fe6430d667056048a5113/xml/vk.xml")
vkg_fetch_vk_xml(VKG_XML_PATH)

# Generate a library target "vkg_lib" from vk.xml + sample.cfg.
# The generator is invoked automatically before this target is built.
# Note: if your config renames the generated headers, you need to also pass the new names here.
vkg_generate(vkg_lib
    XML    ${VKG_XML_PATH}
    CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/sample.cfg
)

add_executable(sample main.cpp)
target_link_libraries(sample PRIVATE vkg_lib)
