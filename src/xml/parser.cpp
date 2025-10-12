/**
 * @file parser.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 12. 10. 2025
 * @date Modified: 12. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include "parser.hpp"
#include <fstream>
#include <cstring>

namespace vkg_gen::xml {
    Dom Parser::parse(const std::string& path) {
        std::ifstream file{ path, std::ios::in };
        if (!file.is_open()) {
            throw std::runtime_error{ "Failed to open file: '" + path + "' because: '" + std::strerror(errno) + "'" };
        }

        Dom dom{ std::string{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() }, nullptr, nullptr, Arena{} };
        return dom;
    };

} // namespace vkg_gen::xml
