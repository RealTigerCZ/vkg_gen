/**
 * @file config.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Config file parser (key=value format)
 *
 * @date Created:  11. 04. 2026
 * @date Modified: 27. 04. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include "config.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ranges>
#include <stdexcept>

ConfigError::ConfigError(const std::string& path, int line, const std::string& message)
    : my_error(path + ":" + std::to_string(line) + ": " + message) {}

ConfigError::ConfigError(const std::string& message)
    : my_error(message) {}

static std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r'))
        s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r'))
        s.remove_suffix(1);
    return s;
}

static bool parse_bool(std::string_view value, bool& out) {
    if (value == "true" || value == "1" || value == "yes") { out = true; return true; }
    if (value == "false" || value == "0" || value == "no") { out = false; return true; }
    return false;
}

bool parse_vulkan_version(std::string_view value, VulkanVersion& out) {
    if (value == "1.0" || value == "VK_VERSION_1_0") { out = VulkanVersion::VK_VERSION_1_0; return true; }
    if (value == "1.1" || value == "VK_VERSION_1_1") { out = VulkanVersion::VK_VERSION_1_1; return true; }
    if (value == "1.2" || value == "VK_VERSION_1_2") { out = VulkanVersion::VK_VERSION_1_2; return true; }
    if (value == "1.3" || value == "VK_VERSION_1_3") { out = VulkanVersion::VK_VERSION_1_3; return true; }
    if (value == "1.4" || value == "VK_VERSION_1_4") { out = VulkanVersion::VK_VERSION_1_4; return true; }
    return false;
}

static bool parse_exception_behavior(std::string_view value, ExceptionBehavior& out) {
    if (value == "ThrowOnly") { out = ExceptionBehavior::ThrowOnly; return true; }
    if (value == "NoThrowOnly") { out = ExceptionBehavior::NoThrowOnly; return true; }
    if (value == "BothWithDefaultThrow") { out = ExceptionBehavior::BothWithDefaultThrow; return true; }
    if (value == "BothWithDefaultNoThrow") { out = ExceptionBehavior::BothWithDefaultNoThrow; return true; }
    if (value == "BothWithoutDefault") { out = ExceptionBehavior::BothWithoutDefault; return true; }
    return false;
}

static bool parse_log_level(std::string_view value, LogLevel& out) {
    if (value == "None") { out = LogLevel::None; return true; }
    if (value == "Error") { out = LogLevel::Error; return true; }
    if (value == "Warning") { out = LogLevel::Warning; return true; }
    if (value == "All") { out = LogLevel::All; return true; }
    return false;
}

static bool parse_compact(std::string_view value, Compact& out) {
    if (value == "Normal") { out = Compact::Normal; return true; }
    if (value == "Compact") { out = Compact::Compact; return true; }
    if (value == "Padded") { out = Compact::Padded; return true; }
    return false;
}

static bool parse_deprecation_behavior(std::string_view value, DeprecationBehavior& out) {
    if (value == "GenerateWithDeprecationWarning") { out = DeprecationBehavior::GenerateWithDeprecationWarning; return true; }
    if (value == "GenerateWithoutDeprecationWarning") { out = DeprecationBehavior::GenerateWithoutDeprecationWarning; return true; }
    if (value == "DontGenerateDeprecatedTypes") { out = DeprecationBehavior::DontGenerateDeprecatedTypes; return true; }
    return false;
}

static bool parse_beta_extensions(std::string_view value, BetaExtensions& out) {
    if (value == "DontGenerate") { out = BetaExtensions::DontGenerate; return true; }
    if (value == "GenerateWithoutProtectMacro") { out = BetaExtensions::GenerateWithoutProtectMacro; return true; }
    if (value == "Generate") { out = BetaExtensions::Generate; return true; }
    return false;
}

static bool parse_extension_filter_mode(std::string_view value, ExtensionFilterMode& out) {
    if (value == "WhiteList") { out = ExtensionFilterMode::WhiteList; return true; }
    if (value == "BlackList") { out = ExtensionFilterMode::BlackList; return true; }
    return false;
}

static bool parse_to_cstr_behavior(std::string_view value, ToCstrFunction& out) {
    if (value == "None") { out = ToCstrFunction::None; return true; }
    if (value == "InCpp") { out = ToCstrFunction::InCpp; return true; }
    if (value == "InHpp") { out = ToCstrFunction::InHpp; return true; }
    return false;
}

void Config::load_from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw ConfigError("Cannot open config file '" + path + "' because: " + std::strerror(errno));

    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        line_num++;
        auto sv = trim(std::string_view(line));
        if (sv.empty() || sv[0] == '#')
            continue;

        auto eq = sv.find('=');
        if (eq == std::string_view::npos)
            throw ConfigError(path, line_num, "Expected key = value, got: " + std::string(sv));

        auto key = trim(sv.substr(0, eq));
        auto value = trim(sv.substr(eq + 1));

#define NON_EMPTY_FIELD(field) (key == #field) {if (value.empty()) throw ConfigError(path, line_num, "Field '" #field "' cannot be empty"); field = value; }
        // String fields
        if NON_EMPTY_FIELD(xml_path)
        else if NON_EMPTY_FIELD(header_path)
        else if NON_EMPTY_FIELD(source_path)
        else if NON_EMPTY_FIELD(detail_namespace)
        else if (key == "namespace") { namespace_name = value; } // Can be empty
        else if (key == "modules_path") { modules_path = value; } // Can be empty

#undef NON_EMPTY_FIELD
        // Version
        else if (key == "target_version") {
            if (!parse_vulkan_version(value, target_version))
                throw ConfigError(path, line_num, "Invalid target_version: " + std::string(value));
        }

        // Enum fields
        else if (key == "exception_behavior") {
            if (value.empty())
                throw ConfigError(path, line_num, "Field 'exception_behavior' cannot be empty");
            if (!parse_exception_behavior(value, exception_behavior))
                throw ConfigError(path, line_num, "Invalid exception_behavior: " + std::string(value));
        } else if (key == "log_level") {
            if (value.empty())
                throw ConfigError(path, line_num, "Field 'log_level' cannot be empty");
            if (!parse_log_level(value, log_level))
                throw ConfigError(path, line_num, "Invalid log_level: " + std::string(value));
        } else if (key == "compact") {
            if (value.empty())
                throw ConfigError(path, line_num, "Field 'compact' cannot be empty");
            if (!parse_compact(value, compact))
                throw ConfigError(path, line_num, "Invalid compact: " + std::string(value));
        } else if (key == "deprecation_behavior") {
            if (value.empty())
                throw ConfigError(path, line_num, "Field 'deprecation_behavior' cannot be empty");
            if (!parse_deprecation_behavior(value, deprecation_behavior))
                throw ConfigError(path, line_num, "Invalid deprecation_behavior: " + std::string(value));
        } else if (key == "beta_extensions") {
            if (value.empty())
                throw ConfigError(path, line_num, "Field 'beta_extensions' cannot be empty");
            if (!parse_beta_extensions(value, beta_extensions))
                throw ConfigError(path, line_num, "Invalid beta_extensions: " + std::string(value));
        } else if (key == "extension_filter") {
            if (value.empty())
                throw ConfigError(path, line_num, "Field 'extension_filter_mode' cannot be empty");
            if (!parse_extension_filter_mode(value, extension_filter_mode))
                throw ConfigError(path, line_num, "Invalid extension_filter_mode: " + std::string(value));
        } else if (key == "to_cstr_behavior") {
            if (value.empty())
                throw ConfigError(path, line_num, "Field 'to_cstr_behavior' cannot be empty");
            if (!parse_to_cstr_behavior(value, to_cstr_behavior))
                throw ConfigError(path, line_num, "Invalid to_cstr_behavior: " + std::string(value));
        }

#define BOOL_FIELD(field) (key == #field) {if (!parse_bool(value, field)) throw ConfigError(path, line_num, "Invalid bool value: " + std::string(value)); }

        // Bool fields
        else if BOOL_FIELD(generate_comments)
        else if BOOL_FIELD(generate_enums_classes)
        else if BOOL_FIELD(generate_enum_numbers)
        else if BOOL_FIELD(generate_handle_class)
        else if BOOL_FIELD(generate_flags_class)
        else if BOOL_FIELD(generate_command_aliases)
        else if BOOL_FIELD(generate_c_type_keywords)
        else if BOOL_FIELD(apply_av1_and_vp9_naming_exceptions)
        else if BOOL_FIELD(generate_extension_defined_macro)
        else if BOOL_FIELD(generate_extension_name_macro)
        else if BOOL_FIELD(generate_extension_version_macro)

            // Extensions
        else if (key == "extensions") {
            std::string err = parse_extensions(value);
            if (!err.empty())
                throw ConfigError(path, line_num, "Extension field '" + std::string(value) + "' is invalid: " + err);
        } else {
            throw ConfigError(path, line_num, "Unknown field: " + std::string(key));
        }
    }
}

std::string Config::parse_extensions(std::string_view value) {
    if (value.empty()) {
        extension_names.clear();
        return "";
    }

    for (auto&& part : value | std::views::split(',')) {
        std::string extension(trim(std::string_view(part)));
        if (extension.empty()) {
            std::cerr << "Warning: " << "Empty extension in config!" << '\n';
            continue;
        }
        if (extension_names.contains(extension)) {
            std::cerr << "Warning: " << "Found duplicate extension in config: " << extension << '\n';
            continue;
        }


        if (std::ranges::all_of(extension, isdigit))
            return "Filtering extensions by number is not supported yet!";
        if (isdigit(extension[0]))
            return "Extension name '" + extension + "' cannot start with a digit! ";
        if (!std::ranges::all_of(extension, [](char c) { return isalnum(c) || c == '_'; }))
            return "Invalid character in extension name '" + extension + "'! Can only contain alphanumeric characters and underscores!";

        extension_names.insert(std::move(extension));
    }

    return "";
}

static constexpr std::string_view DEFAULT_CONFIG =
"# Available modes: BlackList, WhiteList\n"
"extension_filter = BlackList\n"
"# Extensions to generate. Using comma separated list of names you can specify which extensions to generate\n"
"# or use BlackList to generate all extensions except listed if possible\n"
"# Empty or commented extensions with BlackList will result in generating all extensions\n"
"extensions = \n"
"# true, false\n"
"generate_comments = true\n"
"# If false, generates C enums instead of 'enum class'\n"
"generate_enums_classes = true\n"
"# If false, generates C vulkan macros instead of 'Handle<>' classes\n"
"generate_handle_class = true\n"
"# If false, generates C VkFlags(uint32) and VkFlags64(uint64) instead of 'Flags<>' classes\n"
"generate_flags_class = true\n"
"# When false, generator tries to remove all unnecessary enum numbers\n"
"generate_enum_numbers = true\n"
"# When false, generator skips (struct | union | enum) before type name that C requires, but C++ doesn't\n"
"generate_c_type_keywords = false\n"
"# AV1 and VP9 would be translated to \"Av1\" and \"Vp9\" in C++\n"
"apply_av1_and_vp9_naming_exceptions = true\n"
"# If false, generator skippes aliases for commands (they usually only add extension suffix)\n"
"generate_command_aliases = true\n"
"\n"
"# Available modes: Normal, Compact, Padded\n"
"compact = Normal\n"
"# Available modes: None, Error, Warning, All\n"
"log_level = Warning\n"
"# Available modes: GenerateWithDeprecationWarning, GenerateWithoutDeprecationWarning, DontGenerateDeprecatedTypes\n"
"deprecation_behavior = GenerateWithoutDeprecationWarning\n"
"# Available modes: DontGenerate, GenerateWithoutProtectMacro, Generate\n"
"beta_extensions = GenerateWithoutProtectMacro\n"
"# Available modes: ThrowOnly, NoThrowOnly, BothWithDefaultThrow, BothWithDefaultNoThrow, BothWithoutDefault\n"
"exception_behavior = BothWithDefaultThrow\n"
"# Available modes: None, InCpp, InHpp\n"
"to_cstr_behavior = InCpp\n"
"# Available versions: VK_VERSION_1_0, VK_VERSION_1_1, VK_VERSION_1_2, VK_VERSION_1_3, VK_VERSION_1_4 or 1.0, 1.1, 1.2, 1.3, 1.4\n"
"target_version = 1.4\n"
"\n"
"# Every extension defines these 3 macros. If false, they won't be generated\n"
"generate_extension_defined_macro = false\n"
"generate_extension_version_macro = false\n"
"generate_extension_name_macro = false\n"
"\n"
"# If provided with empty string, namespace wont be generated, commenting this out will generate default namespace\n"
"namespace = vk\n"
"detail_namespace = detail\n"
"header_path = vkg.hpp\n"
"source_path = vkg.cpp\n"
"xml_path = vk.xml\n"
"# If provided with empty string, modules file wont be generated, commenting this out will generate default module file\n"
"modules_path = vkg.cppm\n";


void Config::save_to_file(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: '" + path + "' because: '" + std::strerror(errno) + "'");
    file << DEFAULT_CONFIG;
}

void print_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [options]\n"
        << "Options:\n"
        << "  --config <path>   Load config from file (recommended for more control)\n\n"
        << "  --xml <path>      Path to vk.xml (overrides config)\n"
        << "  --header <path>   Output header path (overrides config)\n"
        << "  --source <path>   Output source path (overrides config)\n"
        << "  --version <X.Y>   Target Vulkan version, e.g. 1.3 (overrides config)\n"
        << "  --ext <name>      Enable extension, can be specified multiple times, can contain comma-separated list (overrides config)\n\n"
        << "  --create-config <path>   Creates a config file at the given path with default values.\n\n"
        << "  --help            Show this help\n";
}

CliResult handle_cli(Config& config, int argc, char* argv[]) {
    std::string config_path;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--config") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: --config requires a path argument\n";
                return CliResult::ReturnError;
            }
            config_path = argv[++i];
        }
    }

    if (!config_path.empty())
        config.load_from_file(config_path);

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--config") == 0) {
            ++i; // already handled
        } else if (std::strcmp(argv[i], "--xml") == 0) {
            if (i + 1 >= argc) throw ConfigError("--xml requires a path argument");
            config.xml_path = argv[++i];
        } else if (std::strcmp(argv[i], "--header") == 0) {
            if (i + 1 >= argc) throw ConfigError("--header requires a path argument");
            config.header_path = argv[++i];
        } else if (std::strcmp(argv[i], "--source") == 0) {
            if (i + 1 >= argc) throw ConfigError("--source requires a path argument");
            config.source_path = argv[++i];
        } else if (std::strcmp(argv[i], "--version") == 0) {
            if (i + 1 >= argc) throw ConfigError("--version requires a version argument (e.g. 1.3)");
            const char* v = argv[++i];
            if (!parse_vulkan_version({ v, v + std::strlen(v) }, config.target_version))
                throw ConfigError("Invalid Vulkan version: " + std::string(v) + " (expected 1.0 - 1.4)");
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return CliResult::ReturnSuccess;
        } else if (std::strcmp(argv[i], "--ext") == 0) {
            if (i + 1 >= argc) throw ConfigError("--ext requires a path argument");
            config.extension_filter_mode = ExtensionFilterMode::WhiteList; // default: All extension, --ext provided: only provided extensions
            std::string err = config.parse_extensions(argv[++i]);
            if (!err.empty())
                throw ConfigError("Invalid extension: " + err);
        } else if (std::strcmp(argv[i], "--create-config") == 0) {
            if (i + 1 >= argc) throw ConfigError("--create-config requires a path argument");
            if (argc != 3) throw ConfigError("--create-config must be the only argument");
            config.save_to_file(argv[++i]);
            std::cout << "Created config file at '" << argv[i] << "'\n";
            return CliResult::ReturnSuccess;
        } else {
            std::cerr << "Error: Unknown argument: " << argv[i] << std::endl;
            print_usage(argv[0]);
            return CliResult::ReturnError;
        }
    }
    return CliResult::Ok;
}
