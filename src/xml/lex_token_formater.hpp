/**
 * @file lex_token_formater.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Separate implemementations of std::formatters, so they don't pollute the global namespace
 * @date Created: 13. 10. 2025
 * @date Modified: 13. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once
#include "lexer.hpp"
#include <format>

template <>
struct std::formatter<vkgen::xml::Lexer::TokenType> {
    using TokenType = vkgen::xml::Lexer::TokenType;
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const TokenType& token, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", vkgen::xml::token_to_string(token));
    }
};
