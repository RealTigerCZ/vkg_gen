# @file vkgGenerate.cmake
# @author Jaroslav Hucel (xhucel00@vutbr.cz)
# @brief Cmake integration fucntions for vkgen
# @date Created: 01. 05. 2026
# @date Modified: 01. 05. 2026
#
# @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE


set(VKG_DEFAULT_VK_XML_URL
    "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/refs/heads/main/xml/vk.xml"
    CACHE STRING "URL used by vkg_fetch_vk_xml() when no URL is given")

# vkg_fetch_vk_xml(<out_var>
#     [URL  <url>]    # default: ${VKG_DEFAULT_VK_XML_URL}
#     [DEST <path>]   # default: ${CMAKE_CURRENT_BINARY_DIR}/vk.xml
# )
#
# Downloads vk.xml at configure time and stores the resulting absolute
# path in <out_var> in the calling scope. Skips the download if DEST
# already exists, so reconfigures stay cheap and offline-friendly.
# Pair with vkg_generate(... XML ${<out_var>} ...) when you do not
# want to vendor vk.xml.
function(vkg_fetch_vk_xml _vkg_out_var)
    set(_options)
    set(_one_value URL DEST)
    set(_multi_value)
    cmake_parse_arguments(VKGF "${_options}" "${_one_value}" "${_multi_value}" ${ARGN})

    if(VKGF_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "vkg_fetch_vk_xml: unknown arguments: ${VKGF_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT VKGF_URL)
        set(VKGF_URL "${VKG_DEFAULT_VK_XML_URL}")
    endif()
    if(NOT VKGF_DEST)
        set(VKGF_DEST "${CMAKE_CURRENT_BINARY_DIR}/vk.xml")
    endif()

    get_filename_component(_dest_abs "${VKGF_DEST}" ABSOLUTE)
    get_filename_component(_dest_dir "${_dest_abs}" DIRECTORY)
    file(MAKE_DIRECTORY "${_dest_dir}")

    if(NOT EXISTS "${_dest_abs}")
        message(STATUS "vkg_fetch_vk_xml: downloading ${VKGF_URL} -> ${_dest_abs}")
        file(DOWNLOAD "${VKGF_URL}" "${_dest_abs}"
             STATUS  _vkgf_status
             TLS_VERIFY ON)
        list(GET _vkgf_status 0 _vkgf_code)
        list(GET _vkgf_status 1 _vkgf_msg)
        if(NOT _vkgf_code EQUAL 0)
            file(REMOVE "${_dest_abs}")
            message(FATAL_ERROR "vkg_fetch_vk_xml: download failed (${_vkgf_code}): ${_vkgf_msg}")
        endif()
    endif()

    set(${_vkg_out_var} "${_dest_abs}" PARENT_SCOPE)
endfunction()

# vkg_generate(<name>
#     XML        <path>           # required, path to vk.xml
#     CONFIG     <path>           # required, path to a .cfg file
#     [OUTPUT_DIR <dir>]          # default: ${CMAKE_CURRENT_BINARY_DIR}/<name>
#     [HEADER    <filename>]      # default: vkg.hpp
#     [SOURCE    <filename>]      # default: vkg.cpp
# )
#
# Runs the vkgen generator at build time and produces a STATIC library
# target <name> that the consumer can link against. The generator is
# re-invoked whenever the XML, the config file, or the vkgen binary
# itself changes.
function(vkg_generate _vkg_name)
    set(_options)
    set(_one_value XML CONFIG OUTPUT_DIR HEADER SOURCE)
    set(_multi_value)
    cmake_parse_arguments(VKG "${_options}" "${_one_value}" "${_multi_value}" ${ARGN})

    if(VKG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "vkg_generate: unknown arguments: ${VKG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT VKG_XML)
        message(FATAL_ERROR "vkg_generate(${_vkg_name}): XML <path> is required")
    endif()
    if(NOT VKG_CONFIG)
        message(FATAL_ERROR "vkg_generate(${_vkg_name}): CONFIG <path> is required")
    endif()

    if(NOT VKG_HEADER)
        set(VKG_HEADER "vkg.hpp")
    endif()
    if(NOT VKG_SOURCE)
        set(VKG_SOURCE "vkg.cpp")
    endif()
    if(NOT VKG_OUTPUT_DIR)
        set(VKG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${_vkg_name}")
    endif()

    get_filename_component(_xml_abs    "${VKG_XML}"        ABSOLUTE)
    get_filename_component(_config_abs "${VKG_CONFIG}"     ABSOLUTE)
    get_filename_component(_out_abs    "${VKG_OUTPUT_DIR}" ABSOLUTE)
    file(MAKE_DIRECTORY "${_out_abs}")

    set(_header_abs "${_out_abs}/${VKG_HEADER}")
    set(_source_abs "${_out_abs}/${VKG_SOURCE}")

    add_custom_command(
        OUTPUT  "${_header_abs}" "${_source_abs}"
        COMMAND $<TARGET_FILE:vkgen>
                --config "${_config_abs}"
                --xml    "${_xml_abs}"
                --header "${_header_abs}"
                --source "${_source_abs}"
        DEPENDS "${_xml_abs}" "${_config_abs}" vkgen
        WORKING_DIRECTORY "${_out_abs}"
        COMMENT "Generating ${_vkg_name} (${VKG_HEADER}, ${VKG_SOURCE}) via vkgen"
        VERBATIM
    )

    set_source_files_properties("${_header_abs}" "${_source_abs}" PROPERTIES GENERATED TRUE)

    add_library(${_vkg_name} STATIC "${_source_abs}" "${_header_abs}")
    target_include_directories(${_vkg_name} PUBLIC "${_out_abs}")
    target_compile_features(${_vkg_name} PUBLIC cxx_std_20)

    if(NOT WIN32)
        target_link_libraries(${_vkg_name} PUBLIC ${CMAKE_DL_LIBS})
    endif()
endfunction()
