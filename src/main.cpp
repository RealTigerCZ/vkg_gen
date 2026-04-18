/**
 * @file main.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 30. 07. 2025
 * @date Modified: 18. 04. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include <iostream>
#include "xml/parser.hpp"
#include "xml/lexer.hpp"
#include "generator/generator.hpp"
#include "config.hpp"
#include <set>
#include <fstream>
#include <cstring>
#include <unordered_map>

#if 0
void debug_print_node(const vkg_gen::xml::Node& node, int indent = 0, int max_indent = 0) {
    if (max_indent > 0 && indent > max_indent)
        return;

    if (node.isElement()) {
        auto& elem = node.asElement();
        std::cout << std::string(indent, ' ') << " - Tag: " << elem.tag << '\n';
        if (elem.attrs.size() > 0) {
            std::cout << std::string(indent, ' ') << " - Attrs: ";
            for (auto& attr : elem.attrs) {
                std::cout << attr.name << "=" << attr.value << ", ";
            }
            std::cout << "\n";
        }

        if (elem.children.size() > 0) {
            std::cout << std::string(indent, ' ') << " - Children: " << elem.children.size() << "\n";

            for (auto& child : elem.children) {
                debug_print_node(*child, indent + 2, max_indent);
            }
        }
    }
}

std::string node_signature(const vkg_gen::xml::Node& node) {
    if (!node.isElement()) return "TEXT";
    auto& e = node.asElement();

    std::string sig = std::string(e.tag);
    if (e.children.size() == 0) return sig;

    sig += "(";
    for (auto& child : e.children) {
        if (child != e.children[0]) // pointer equality
            sig += ",";
        if (child->isElement())
            sig += std::string(child->asElement().tag);
        else
            sig += "TEXT";
    }
    sig += ")";
    return sig;
}


void debug_print_node_short(const vkg_gen::xml::Node& node, int indent = 0) {
    if (!node.isElement()) return;
    const auto& elem = node.asElement();

    std::cout << std::string(indent, ' ') << "- <" << elem.tag << ">\n";

    // Count identical simple children and structural patterns
    struct Summary {
        std::string tag;
        std::string sig;
        int count = 0;
        const vkg_gen::xml::Node* sample = nullptr;
    };
    std::unordered_map<std::string, Summary> patterns;

    for (auto& child : elem.children) {
        if (!child->isElement()) continue;
        const auto& ch = child->asElement();

        std::string sig = node_signature(*child);
        auto& summary = patterns[sig];
        summary.tag = std::string(ch.tag);
        summary.sig = std::move(sig);
        summary.count++;
        if (!summary.sample) summary.sample = child;
    }

    // Print summaries
    for (auto& [_, summary] : patterns) {
        if (summary.count > 1) {
            std::cout << std::string(indent + 2, ' ')
                << "- <" << summary.sig << "> x " << summary.count << '\n';
        } else if (summary.sample) {
            // Only one of this structure: recurse into it
            debug_print_node_short(*summary.sample, indent + 2);
        }
    }
}

void debug_print(const vkg_gen::xml::Dom& dom) {
    std::cout << "File: " << FILE_PATH << '\n';
    std::cout << "Header: " << '\n';
    std::cout << " - version: " << dom.header->version << '\n';
    std::cout << " - encoding: " << dom.header->encoding << '\n';
    std::cout << " - standalone: " << dom.header->standalone << '\n';
    std::cout << "Root: " << '\n';
    debug_print_node_short(*dom.root);
}
#endif

inline std::ofstream open_or_throw(const std::string& path, std::ios::openmode mode) {
    std::ofstream file{ path, mode };
    if (!file.is_open())
        throw std::runtime_error{ "Failed to open file: '" + path + "' because: '" + std::strerror(errno) + "'" };
    return file;
}

void test(Config& config, vkg_gen::xml::Dom& dom) {
    using namespace vkg_gen::xml;

    std::ofstream header = open_or_throw(config.header_path, std::ios::out);
    std::ofstream source = open_or_throw(config.source_path, std::ios::out);

    vkg_gen::Generator::Generator generator;
    generator.generate(dom, header, source, config);
}

int handle_cli(Config& config, int argc, char* argv[]) {
    std::string config_path;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--config") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: --config requires a path argument\n";
                return 1;
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
        } else if (std::strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 1;
        } else if (std::strcmp(argv[i], "--ext") == 0) {
            if (i + 1 >= argc) throw ConfigError("--ext requires a path argument");
            std::string err = config.parse_extensions(argv[++i]);
            if (!err.empty())
                throw ConfigError("Invalid extension: " + err);
        } else if (std::strcmp(argv[i], "--create-config") == 0) {
            if (i + 1 >= argc) throw ConfigError("--create-config requires a path argument");
            if (argc != 3) throw ConfigError("--create-config must be the only argument");
            config.save_to_file(argv[++i]);
            std::cout << "Created config file at '" << argv[i] << "'\n";
            return 1;
        } else {
            std::cerr << "Error: Unknown argument: " << argv[i] << std::endl;
            print_usage(argv[0]);
            return 2;
        }
    }
    return 0;
}

void check_config(const Config& config) {
    // Fully unsupported options
    if (config.compact != Compact::Normal)
        std::cout << "Warning: 'compact' is not supported yet\n";
    if (config.header_only)
        std::cout << "Warning: 'header_only' is not supported yet\n";
    if (config.generate_extension_defined_macro)
        std::cout << "Warning: 'generate_extension_defined_macro' is not supported yet\n";
    if (!config.generate_enum_numbers)
        std::cout << "Warning: 'generate_enum_numbers = false' is not supported yet\n";
    if (!config.generate_c_type_keywords)
        std::cout << "Warning: 'generate_c_type_keywords = false' is not supported yet\n";

    // Partially working options
    if (config.deprecation_behavior == DeprecationBehavior::DontGenerateDeprecatedTypes)
        std::cout << "Warning: 'DontGenerateDeprecatedTypes' is not supported yet (deprecated types will still be generated)\n";
    if (config.exception_behavior == ExceptionBehavior::NoThrowOnly ||
        config.exception_behavior == ExceptionBehavior::BothWithDefaultNoThrow)
        std::cout << "Warning: Boilerplate functions (loadLib, initInstance, initDevice) always use throw variant regardless of exception_behavior\n";

    if (config.generate_handle_class == false)
        std::cout << "Warning: 'generate_handle_class = false' is not supported yet - project moved to better support C++ in a first place\n";
    if (config.generate_enums_classes == false)
        std::cout << "Warning: 'generate_enums_classes = false' may not work - project moved to better support C++ in a first place\n";

}

int main(int argc, char* argv[]) {
    namespace xml = vkg_gen::xml;
    Config config;
    try {
        if (int ret = handle_cli(config, argc, argv) != 0)
            return ret == 1 ? 0 : 1;

        check_config(config);

        xml::Parser parser;
        auto dom = parser.parse(config.xml_path);
        test(config, dom);
    }
    catch (const xml::Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (const ConfigError& e) {
        std::cerr << "Config error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
