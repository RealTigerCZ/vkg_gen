/**
 * @file xml_parser.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 31. 07. 2025
 * @date Modified: 29. 09. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */


#include "xml_parser.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <assert.h>
#include <sstream>

namespace vkg_gen {
    XmlLexTokenType XmlLexer::next(Expected expected) {
        m_last_value = { (int)m_buffer.size(), 0 };

        if (*m_ptr == 0)
            return XmlLexTokenType::EndOfFile;

        switch (expected) {
        case Expected::Header:
            // CHECK: skip whitespace?
            if (std::distance(m_ptr, m_data.end().base()) < 5)
                throw LexerError{ *this, "Unexpected end of data",  std::distance(m_ptr, m_data.end().base()) };

            if (std::string_view{ m_ptr, 5 } != "<?xml") {
                throw LexerError{ *this,  "Expected '<?xml' tag.", 5 };
            }

            m_ptr += 5;
            return XmlLexTokenType::XmlTagStart;

        case Expected::Text:
            return load_text();

        case Expected::Tag:
            return load_identifier();

        case Expected::Attribute:
            skip_whitespace();
            switch (*m_ptr) {
            case 0:
                return XmlLexTokenType::EndOfFile;
            case '=':
                ++m_ptr;
                return XmlLexTokenType::Equals;

            case '"': // fallthrough
            case '\'':
                return load_string();
            case '?':
                if (m_ptr[1] == '>') {
                    m_ptr += 2;
                    return XmlLexTokenType::XmlTagEnd;
                }
                throw LexerError{ *this, "Expected '?>' tag.", 2 };
            case '/':
                if (m_ptr[1] == '>') {
                    m_ptr += 2;
                    return XmlLexTokenType::SlashGreaterThan;
                }
                throw LexerError{ *this, "Expected '/>' tag.", 2 };

            case '>':
                ++m_ptr;
                return XmlLexTokenType::GreaterThan;

            case '<':
                if (std::distance(m_ptr, m_data.end().base()) < 4) // CHECK: off by one
                    throw LexerError{ *this, "Unexpected end of data",  std::distance(m_ptr, m_data.end().base()) };
                if (m_ptr[1] != '!' || m_ptr[2] != '-' || m_ptr[3] != '-')
                    throw LexerError{ *this, "Expected atribute or '<!--' tag.", 4 };
                remove_comment();
                return next(Expected::Attribute);

            default:
                return load_identifier();
            }

        }
    }


    XmlLexTokenType XmlLexer::load_text() {
        skip_whitespace();
        if (*m_ptr == 0)
            return XmlLexTokenType::EndOfFile;


        do {
            switch (*m_ptr) {
            case '<':
                if ((std::distance(m_ptr, m_data.end().base()) >= 4) && m_ptr[1] == '!' && m_ptr[2] == '-' && m_ptr[3] == '-') {
                    remove_comment();
                    break;
                }
                m_last_value.size = m_buffer.size() - m_last_value.start;
                if (m_last_value.size > 0)
                    return XmlLexTokenType::Text;

                if (m_ptr[1] == '/') {
                    m_ptr += 2;
                    return XmlLexTokenType::LessThanSlash;
                }

                ++m_ptr;
                return XmlLexTokenType::LessThan;

            case '&':
                escape_entity();
                break;
            case ']':
                if (std::distance(m_ptr, m_data.end().base()) >= 3 && m_ptr[1] == ']' && m_ptr[2] == '>') {
                    throw std::runtime_error{ "TODO: CDATA section not supported yet" };
                }
                m_buffer.push_back(*m_ptr);
                break;
            default:
                handle_new_line();
                m_buffer.push_back(*m_ptr);
            }
        } while (*(++m_ptr));

        return XmlLexTokenType::EndOfFile;
    }

    XmlLexTokenType XmlLexer::load_identifier() {
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
            return XmlLexTokenType::EndOfFile;

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
            m_buffer.push_back(*m_ptr);
        } while (*(++m_ptr));

        if (m_ptr == start)
            throw LexerError{ *this, "Expected identifier. But found '" + std::string(m_ptr, 1) + "'" };

        m_last_value.size = m_buffer.size() - m_last_value.start;
        return XmlLexTokenType::Identifier;
    }


    XmlLexTokenType XmlLexer::load_string() {
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
        m_last_value.size = m_buffer.size() - m_last_value.start;
        return XmlLexTokenType::String;
    }
    void XmlLexer::escape_entity() {
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


        std::string_view entity{ m_ptr, end - m_ptr }; // Remove ';'

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
            throw LexerError{ *this, "Unknown entity '" + std::string{ entity } + "'", entity.length() };
        }

        m_buffer.push_back(it->second);
        m_ptr = end;
    }

    inline void XmlLexer::handle_new_line() {
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


    void XmlLexer::remove_comment() {
        while (*m_ptr && (m_ptr[0] != '-' || m_ptr[1] != '-' || m_ptr[2] != '>')) {
            handle_new_line();
            ++m_ptr;
        }
        m_ptr += 3;
        if (m_ptr >= m_data.data() + m_data.length()) {
            throw LexerError{ *this, "Unterminated comment" };
        }
    }

    void XmlLexer::skip_whitespace() {
        while (*m_ptr && (*m_ptr == '\n' || *m_ptr == '\r' || *m_ptr == '\t' || *m_ptr == ' ')) {
            handle_new_line();
            ++m_ptr;
        }
    }

    XmlDom XmlParser::parse(const std::string& path) {
        std::ifstream file{ path, std::ios::in };
        if (!file.is_open()) {
            throw std::runtime_error{ "Failed to open file: '" + path + "' because: '" + std::strerror(errno) + "'" };
        }

        XmlFile xml_file{ std::string{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() }, path };

        XmlDom dom{ XmlHeader{ "1.0", "UTF-8" }, XmlItem{ "root", "" }, xml_file };
        return dom;
    };

    std::ostream& operator<<(std::ostream& os, XmlLexTokenType type) {
        switch (type) {
        case XmlLexTokenType::LessThan:
            return os << "<";
        case XmlLexTokenType::GreaterThan:
            return os << ">";
        case XmlLexTokenType::SlashGreaterThan:
            return os << "/>";
        case XmlLexTokenType::LessThanSlash:
            return os << "</";
        case XmlLexTokenType::Equals:
            return os << "=";
        case XmlLexTokenType::Identifier:
            return os << "Identifier";
        case XmlLexTokenType::Text:
            return os << "Text";
        case XmlLexTokenType::XmlTagStart:
            return os << "<?xml";
        case XmlLexTokenType::XmlTagEnd:
            return os << "?>";
        case XmlLexTokenType::EndOfFile:
            return os << "EOF";
        case XmlLexTokenType::String:
            return os << "String";
        }

        UNREACHABLE();
    }

    LexerError::LexerError(const XmlLexer& lexer, const std::string& msg, int len) :
        std::runtime_error(
            [&] {
                std::stringstream ss;
                auto pos = lexer.get_pos();
                ss << "Error occured when lexing at " << lexer.file_path << ':' << pos.line << ':' << pos.col << '\n' <<
                    msg << '\n';
                sv line(lexer.m_last_line_end + 1, lexer.m_data.end().base() - lexer.m_last_line_end);
                size_t new_size = line.find_first_of('\n');
                if (new_size != sv::npos)
                    line = line.substr(0, new_size);

                int error_loc = std::distance(lexer.m_last_line_end + 1, lexer.m_ptr);
                ss << pos.line << " | " << line << '\n';
                ss << pos.line << " | " << std::string(error_loc, ' ') << std::string(len, '^') << "~~~here" << std::endl;
                return ss.str();
            }()) {};
}

