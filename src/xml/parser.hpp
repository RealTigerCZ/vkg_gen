/**
 * @file parser.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 12. 10. 2025
 * @date Modified: 1. 11. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once
#include "xml.hpp"



namespace vkg_gen::xml {
    class Lexer;

    class Parser {
        const char* file_path;

        void parse(Dom& dom, Lexer& lexer);
        bool load_attr(Lexer& lexer, vec<Attribute>& attrs, Dom& dom);
        void load_header(Lexer& lexer, Dom& dom);

    public:
        Dom parse(const std::string& path);

        Parser() = default;
        ~Parser() = default;
    };

    class ParserError : public Error {
        enum class TokenType;
    public:
        constexpr static int USE_TOKEN_LEN = -1;

        ParserError(const Lexer& lexer, const std::string& msg, int len = USE_TOKEN_LEN);
    };
} // namespace vkg_gen::xml
