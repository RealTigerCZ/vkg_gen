/**
 * @file config.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Program configuration loader
 *
 * @date Created:  25. 02. 2026
 * @date Modified: 18. 04. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once
#include "debug_macros.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>

class ConfigError : public my_error {
public:
    ConfigError(const std::string& path, int line, const std::string& message);
    ConfigError(const std::string& message);
};

enum class LogLevel : uint8_t {
    None,
    Error,
    Warning,
    All,
};

enum class DeprecationBehavior : uint8_t {
    GenerateWithDeprecationWarning,
    GenerateWithoutDeprecationWarning,
    DontGenerateDeprecatedTypes,
};

enum class Compact : uint8_t {
    Normal,
    Compact,
    Padded
};

enum class VulkanVersion : uint8_t {
    VK_VERSION_1_0,
    VK_VERSION_1_1,
    VK_VERSION_1_2,
    VK_VERSION_1_3,
    VK_VERSION_1_4,
};

constexpr const char* vulkan_version_to_string(VulkanVersion v) {
    switch (v) {
    case VulkanVersion::VK_VERSION_1_0: return "VK_VERSION_1_0";
    case VulkanVersion::VK_VERSION_1_1: return "VK_VERSION_1_1";
    case VulkanVersion::VK_VERSION_1_2: return "VK_VERSION_1_2";
    case VulkanVersion::VK_VERSION_1_3: return "VK_VERSION_1_3";
    case VulkanVersion::VK_VERSION_1_4: return "VK_VERSION_1_4";
    }
    UNREACHABLE();
}

enum class STLClassesInfo : uint8_t {
    UseStdImpl, // include <vector> and use std::vector/...
    UseOwnImpl, // use own implementation
    DetectIfImplNeeded // Dont include <vector> but use it if its already included, also generate own implementation for fall back
};

enum class ExceptionBehavior : uint8_t {
    ThrowOnly,              // Generate only throwing functions, without suffix
    NoThrowOnly,            // Generate only nothrow functions, without suffix
    BothWithDefaultThrow,   // Generate throw and nothrow functions with suffixes, generate additional default function that calls throw function
    BothWithDefaultNoThrow, // Generate throw and nothrow functions with suffixes, generate additional default function that calls nothrow function
    BothWithoutDefault      // Generate throw and nothrow functions with suffixes
};

enum class BetaExtensions : uint8_t {
    DontGenerate,
    GenerateWithoutProtectMacro,
    Generate,
};

enum class ToCstrFunction : uint8_t {
    None,  // Don't generate to_cstr()
    InCpp, // Generate declarations in hpp but define it in cpp
    InHpp, // Generate inline to_cstr() in hpp
};

enum class ExtensionFilterMode : uint8_t {
    WhiteList,
    BlackList,
};

struct Config {
    ExtensionFilterMode extension_filter_mode = ExtensionFilterMode::BlackList;
    std::unordered_set<std::string> extension_names = {};

    bool generate_comments = true;
    bool generate_enums_classes = true; // false means generate C enums
    bool generate_handle_class = true;  // false means generate C vulkan macros
    bool generate_flags_class = true;   // false means generate C VkFlags (uint32) and VkFlags64 (uint64)
    bool generate_enum_numbers = true;  // false tries to remove all unnecessary enum numbers
    bool generate_c_type_keywords = false; // C requires (struct|union|enum) before type name, C++ doesn't
    bool apply_av1_and_vp9_naming_exceptions = true; // AV1 and VP9 would be translated to "Av1" and "Vp9" in C++
    bool generate_command_aliases = true; // false means skipping aliases for commands, that usually only adds extension suffix

    Compact compact = Compact::Normal;
    LogLevel log_level = LogLevel::Warning;
    DeprecationBehavior deprecation_behavior = DeprecationBehavior::GenerateWithoutDeprecationWarning;
    BetaExtensions beta_extensions = BetaExtensions::GenerateWithoutProtectMacro;
    ExceptionBehavior exception_behavior = ExceptionBehavior::BothWithDefaultThrow;
    ToCstrFunction to_cstr_behavior = ToCstrFunction::InCpp;
    VulkanVersion target_version = VulkanVersion::VK_VERSION_1_4;

    // C++ features: namespacing?
    // C++ features: modules?

    bool generate_extension_defined_macro = false;
    bool generate_extension_name_macro = false;
    bool generate_extension_version_macro = false;

    bool header_only = false;
    std::string header_only_guard = "VKG_IMPLEMENTATION";

    std::string namespace_name = "vk"; // empty means no namespace
    std::string header_path = "vkg.hpp";
    std::string source_path = "vkg.cpp";
    std::string xml_path = "vk.xml";

    /* Currently unsupported, maybe replaced by "converters" in future
        STLClassesInfo vector = STLClassesInfo::UseStdImpl; // use std::vector or own implementation?
        STLClassesInfo iterator = STLClassesInfo::UseStdImpl;
        STLClassesInfo string_view = STLClassesInfo::UseStdImpl;
    */

    // Future ideas for options:
    // - bool  generate_unique_handles
    // - PhysicalDeviceBehavior physical_device_behavior = ::AlwaysSkip, ::AlwaysRequire, ::GenerateBoth

    void load_from_file(const std::string& path);
    std::string parse_extensions(std::string_view value);
    void save_to_file(const std::string& path);
};

enum class CliResult : uint8_t {
    Ok,
    ReturnSuccess,
    ReturnError,
};

bool parse_vulkan_version(std::string_view value, VulkanVersion& out);
CliResult handle_cli(Config& config, int argc, char* argv[]);
void print_usage(const char* program_name);
