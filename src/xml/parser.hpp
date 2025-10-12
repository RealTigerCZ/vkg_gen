/**
 * @file parser.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 12. 10. 2025
 * @date Modified: 12. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once
#include "xml.hpp"

namespace vkg_gen::xml {

    class Parser {

    public:
        Dom parse(const std::string& path);

        Parser() = default;
        ~Parser() = default;
    };

} // namespace vkg_gen::xml
