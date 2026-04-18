/**
 * @file lexer.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Implementation of Lexer (tokenizer)
 *
 * @date Created: 31. 07. 2025
 * @date Modified: 10. 04. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */


#include "lexer.hpp"

#include "../arena.hpp"
#include "../debug_macros.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>

namespace vkgen::xml {
    Lexer::TokenType Lexer::next(Expected expected) {
        auto token = _next(expected); // FIXME: refactor
        if (m_buffer.size() == 0)
            m_last_value = { m_token_start, static_cast<size_t>(m_ptr - m_token_start) }; // TODO: what about wchar?
        else {
            m_last_value = { m_buffer.data(), m_buffer.size() };
        }

        return token;
    }

    Lexer::TokenType Lexer::_next(Expected expected) {
        m_buffer.clear();
        m_token_start = m_ptr;

        if (*m_ptr == 0)
            return TokenType::EndOfFile;

        switch (expected) {
        case Expected::Header:
            // CHECK: skip whitespace?
            if (std::distance(m_ptr, m_data.data() + m_data.size()) < 5)
                throw LexerError{ *this, "Unexpected end of data",  (int)std::distance(m_ptr, m_data.data() + m_data.size()) };

            if (std::string_view{ m_ptr, 5 } != "<?xml") {
                throw LexerError{ *this,  "Missing '<?xml' tag.", 5 };
            }

            m_ptr += 5;
            return TokenType::XmlTagStart;

        case Expected::Text:
            return load_text();

        case Expected::Tag:
            return load_identifier();

        case Expected::Attribute:
            // TODO: Move into load_attribute
            skip_whitespace();
            switch (*m_ptr) {
            case 0:
                return TokenType::EndOfFile;
            case '=':
                ++m_ptr;
                return TokenType::Equals;

            case '"': // fallthrough
            case '\'':
                return load_string();
            case '?':
                if (m_ptr[1] == '>') {
                    m_ptr += 2;
                    return TokenType::XmlTagEnd;
                }
                throw LexerError{ *this, "Expected '?>' tag.", 2 };
            case '/':
                if (m_ptr[1] == '>') {
                    m_ptr += 2;
                    return TokenType::SlashGreaterThan;
                }
                throw LexerError{ *this, "Expected '/>' tag.", 2 };

            case '>':
                ++m_ptr;
                return TokenType::GreaterThan;

            case '<':
                if (std::distance(m_ptr, m_data.data() + m_data.size()) < 4) // CHECK: off by one
                    throw LexerError{ *this, "Unexpected end of data",  (int)std::distance(m_ptr, m_data.data() + m_data.size()) };
                if (m_ptr[1] != '!' || m_ptr[2] != '-' || m_ptr[3] != '-')
                    throw LexerError{ *this, "Expected atribute or '<!--' tag.", 4 };
                remove_comment();
                return next(Expected::Attribute);

            default:
                return load_identifier();
            }

        }

        UNREACHABLE();
    }


    Lexer::TokenType Lexer::load_text() {
        skip_whitespace();
        if (*m_ptr == 0)
            return TokenType::EndOfFile;


        do {
            switch (*m_ptr) {
            case '<':
                if ((std::distance(m_ptr, m_data.data() + m_data.size()) >= 4) && m_ptr[1] == '!' && m_ptr[2] == '-' && m_ptr[3] == '-') {
                    remove_comment();
                    break;
                }
                if (m_buffer.size() > 0)
                    return TokenType::Text;

                if (m_ptr[1] == '/') {
                    m_ptr += 2;
                    return TokenType::LessThanSlash;
                }

                ++m_ptr;
                return TokenType::LessThan;

            case '&':
                escape_entity();
                break;
            case ']':
                if (std::distance(m_ptr, m_data.data() + m_data.size()) >= 3 && m_ptr[1] == ']' && m_ptr[2] == '>') {
                    throw std::runtime_error{ "TODO: CDATA section not supported yet" };
                }
                m_buffer.push_back(*m_ptr);
                break;
            default:
                handle_new_line();
                m_buffer.push_back(*m_ptr);
            }
        } while (*(++m_ptr));

        return TokenType::EndOfFile;
    }

    Lexer::TokenType Lexer::load_identifier() {
        // FIXME: this is not correct for UTF-8, but it works for windows-1252
        // From XML spec: NameStartChar ::= ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#xFF]
        const auto isNameStartChar = [](char c) {
            return c == ':' || (c >= 'A' && c <= 'Z') || c == '_' || (c >= 'a' && c <= 'z') ||
                (c >= '\xC0' && c <= '\xD6') || (c >= '\xD8' && c <= '\xF6') || (c >= '\xF8' && c <= '\xFF');
            };

        // From XML spec: NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7
        const auto isNameChar = [&isNameStartChar](char c) {
            return isNameStartChar(c) || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '\xB7';
            };

        if (*m_ptr == 0)
            return TokenType::EndOfFile;

        auto start = m_ptr;

        do {
            if ((unsigned)*m_ptr >= 0x80)
                throw LexerError{ *this, "Identifiers must be in ASCII range" };

            if (m_ptr == start) {
                if (!isNameStartChar(*m_ptr))
                    break;
            } else {
                if (!isNameChar(*m_ptr))
                    break;
            }

        } while (*(++m_ptr));

        if (m_ptr == start)
            throw LexerError{ *this, "Expected identifier. But found '" + std::string(m_ptr, 1) + "'" };

        return TokenType::Identifier;
    }


    Lexer::TokenType Lexer::load_string() {
        assert(*m_ptr == '"' || *m_ptr == '\'');

        auto start = m_ptr++; // quote or apostrophe
        do {
            if (*m_ptr == *start) // closing quote or apostrophe
                break;
            if (*m_ptr == '&') {
                escape_entity();
                continue;
            }
            if (*m_ptr == '<')
                throw LexerError{ *this, "'<' is not allowed in attribute value" };

            handle_new_line();

            m_buffer.push_back(*m_ptr);
        } while (*(++m_ptr));
        ++m_ptr;
        return TokenType::String;
    }

    void Lexer::escape_entity() {
        static const std::unordered_map<std::string_view, char> entities{
            { "lt", '<' },
            { "gt", '>' },
            { "amp", '&' },
            { "apos", '\'' },
            { "quot", '"' }
        };
        constexpr int longest_entity = 4; // quot and #x7F and #255 are the longest
        ++m_ptr; // remove '&'

        auto end = std::find_if(m_ptr, m_ptr + longest_entity + 1, [](char c) { return c == ';'; });
        if (end == m_ptr + longest_entity + 1)
            throw LexerError{ *this, "TODO: incomplete entity" };


        std::string_view entity{ m_ptr, static_cast<size_t>(end - m_ptr) }; // Remove ';'

        if (entity[0] == '#') {
            if (entity[1] == 'x') {
                entity.remove_prefix(2);
                m_buffer.push_back(std::stoul(entity.data(), nullptr, 16));
            } else {
                entity.remove_prefix(1);
                m_buffer.push_back(std::stoul(entity.data(), nullptr, 10));
            }
            m_ptr = end;
            return;
        }


        auto it = entities.find(entity);
        if (it == entities.end()) {
            throw LexerError{ *this, "Unknown entity '" + std::string{ entity } + "'", (int)entity.length() };
        }

        m_buffer.push_back(it->second);
        m_ptr = end;
    }

    inline void Lexer::handle_new_line() {
        if (*m_ptr == '\r') {
            ++m_line;
            m_last_line_end = m_ptr;
            // If next char is '\n', skip it too (Windows newline)
            if (*(m_ptr + 1) == '\n') ++m_ptr;
        } else if (*m_ptr == '\n') {
            ++m_line;
            m_last_line_end = m_ptr;
        }
    }


    void Lexer::remove_comment() {
        while (*m_ptr && (m_ptr[0] != '-' || m_ptr[1] != '-' || m_ptr[2] != '>')) {
            handle_new_line();
            ++m_ptr;
        }
        m_ptr += 3;
        if (m_ptr >= m_data.data() + m_data.length()) {
            throw LexerError{ *this, "Unterminated comment" };
        }
        m_token_start = m_ptr;
    }

    void Lexer::skip_whitespace() {
        while (*m_ptr && (*m_ptr == '\n' || *m_ptr == '\r' || *m_ptr == '\t' || *m_ptr == ' ')) {
            handle_new_line();
            ++m_ptr;
        }
        m_token_start = m_ptr;
    }

    sv token_to_string(Lexer::TokenType type) noexcept {
        using Type = Lexer::TokenType;
        switch (type) {
        case Type::LessThan:         return "<";
        case Type::GreaterThan:      return ">";
        case Type::SlashGreaterThan: return "/>";
        case Type::LessThanSlash:    return "</";
        case Type::Equals:           return "=";
        case Type::Identifier:       return "ID";
        case Type::Text:             return "TEXT";
        case Type::XmlTagStart:      return "<?xml";
        case Type::XmlTagEnd:        return "?>";
        case Type::EndOfFile:        return "EOF";
        case Type::String:           return "STR";
        }

        UNREACHABLE_QUIET();
        return "UNKNOWN";
    }

    std::ostream& operator<<(std::ostream& os, Lexer::TokenType type) {
        return os << token_to_string(type);
    }


    LexerError::LexerError(const Lexer& lexer, const std::string& msg, int len) :
        Error(lexer.get_err_loc(), msg, "lexing", len, false) {}
} // namespace vkg_gen::xml


namespace vkgen {
    using sv = std::string_view;
    sv xml::Lexer::get_and_save_value(Arena& arena) {
        return arena.storeString(get_value());
    }

}
