/**
 * @file main.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Entry point; loads config, parses vk.xml, and runs the generator.
 * @date Created: 30. 07. 2025
 * @date Modified: 18. 04. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include "config.hpp"
#include "generator/generator.hpp"
#include "xml/parser.hpp"

#include <cstring>
#include <fstream>
#include <iostream>


static std::ofstream open_or_throw(const std::string& path, std::ios::openmode mode) {
    std::ofstream file{ path, mode };
    if (!file.is_open())
        throw std::runtime_error{ "Failed to open file: '" + path + "' because: '" + std::strerror(errno) + "'" };
    return file;
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
    if (config.generate_c_type_keywords)
        std::cout << "Warning: 'generate_c_type_keywords = true' is not supported yet\n";

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
    namespace xml = vkgen::xml;
    try {
        Config config;
        CliResult res = handle_cli(config, argc, argv);
        if (res != CliResult::Ok)
            return res == CliResult::ReturnError ? 1 : 0;

        check_config(config);

        xml::Parser parser;
        xml::Dom dom = parser.parse(config.xml_path);

        std::ofstream header = open_or_throw(config.header_path, std::ios::out);
        std::ofstream source = open_or_throw(config.source_path, std::ios::out);

        vkgen::Generator::Generator generator;
        generator.generate(dom, header, source, config);
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
