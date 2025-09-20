/**
 * @file xml_parser.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief
 * @date Created: 31. 07. 2025
 * @date Modified: 20. 09. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */


#include "xml_parser.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>

namespace vkg_gen {
    XmlLexTokenType XmlLexer::next(bool skip_whitespace, bool allow_text) {
        m_last_value = { m_ptr, 0 };
        if (skip_whitespace)
            this->skip_whitespace();

        if (*m_ptr == 0) {
            return XmlLexTokenType::EndOfFile;
        }

        switch (*(m_ptr++)) {
        case '<':
            // Handle '</'
            if (m_ptr[0] == '/') {
                ++m_ptr;
                return XmlLexTokenType::LessThanSlash;
                // Handle '<?xml'
            } else if (m_ptr[0] == '?') {
                if ((m_ptr + 4) < m_data.data() + m_data.length()) { // CHECK: off-by-one
                    if (m_ptr[1] == 'x' && m_ptr[2] == 'm' && m_ptr[3] == 'l') {
                        m_ptr += 4;
                        return XmlLexTokenType::XmlTagStart;
                    }
                }
                throw std::runtime_error{ "TODO: invalid ?xml tag" };
                // Handle '<!--'
            } else if (m_ptr[0] == '!') {
                if ((m_ptr + 2) < m_data.data() + m_data.length()) { // CHECK: off-by-one
                    if (m_ptr[1] == '-' && m_ptr[2] == '-') {
                        m_ptr += 3;
                        remove_comment();
                        return next(skip_whitespace, allow_text);
                    }
                }
                throw std::runtime_error{ "TODO: invalid comment" };
            }
            return XmlLexTokenType::LessThan;
        case '>':
            return XmlLexTokenType::GreaterThan;
        case '/':
            if (m_ptr[0] == '>') {
                ++m_ptr;
                return XmlLexTokenType::SlashGreaterThan;
            }
            break;
        case '=':
            return XmlLexTokenType::Equals;
        case '"':
            return XmlLexTokenType::Quote;
        case '?':
            if (m_ptr[0] == '>')
                return XmlLexTokenType::XmlTagEnd;
            break;
        }

        m_ptr--; // put the character back
        auto tok = load_text(allow_text);
        m_last_value = { m_last_value.data(), m_ptr - m_last_value.data() };
        return tok;


        // TODO: refactor the code that repeats
    }

    XmlLexTokenType XmlLexer::load_text(bool allow_text) {
        // TODO: striping spaces

        // Zero length identifier
        if (*m_ptr == 0)
            return XmlLexTokenType::EndOfFile;


        do {
            switch (*m_ptr) {
            case '<': // fallthrough
            case '>': // fallthrough
            case '=': // fallthrough
            case '"':
                return allow_text ? XmlLexTokenType::Text : XmlLexTokenType::Identifier;

            case '/': // fallthrough
            case '?':
                // '/' or '?' can be used in text but if it's part of other token (/> or ?>), then the text ends
                if (m_ptr[1] == '>') {
                    return allow_text ? XmlLexTokenType::Text : XmlLexTokenType::Identifier;
                }

                if (!allow_text)
                    return XmlLexTokenType::Text; // Identifier cannot contain '?' or '/'

                break;
            case '&':
                if (!allow_text)
                    return XmlLexTokenType::Text; // Identifier cannot contain '&'

                std::cout << __FILE__ << ":" << __LINE__ << ": " << "TODO: & escaping" << std::endl;
                while (*(++m_ptr) != ';' && *m_ptr != 0); // skip the rest of the entity
                break;

                // TODO: windows newlines
            case '\n':  // fallthrough
                if (!allow_text) { // Identifier ends with any whitespace
                    return XmlLexTokenType::Identifier;
                }
                m_line++;
                m_last_line_end = m_ptr;
            case '\t':  // fallthrough
            case ' ':
                if (!allow_text) { // Identifier ends with any whitespace
                    return XmlLexTokenType::Identifier;
                }
                break;

            default:
                // Check the valid identifier characters if text is not allowed
                if (!allow_text) {
                    if (m_last_value.data() == m_ptr) { // first character
                        if (!((*m_ptr >= 'a' && *m_ptr <= 'z')
                            || (*m_ptr >= 'A' && *m_ptr <= 'Z')
                            || *m_ptr == '_')
                            )
                            return XmlLexTokenType::Text; // Identifier cannot start with not allowed character

                    } else // not first character
                        if (!((*m_ptr >= 'a' && *m_ptr <= 'z')
                            || (*m_ptr >= 'A' && *m_ptr <= 'Z')
                            || (*m_ptr >= '0' && *m_ptr <= '9')
                            || *m_ptr == '_' || *m_ptr == '.')
                            )
                            return XmlLexTokenType::Text; // Identifier cannot contain not allowed character
                }

                // TODO: remove this debug:
                if (!((*m_ptr >= 'a' && *m_ptr <= 'z')
                    || (*m_ptr >= 'A' && *m_ptr <= 'Z')
                    || (*m_ptr >= '0' && *m_ptr <= '9')
                    || *m_ptr == '_' || *m_ptr == '.')
                    ) {
                    static const std::string allowed_chars{ "*.,'#|()[]{}~:-+@\\" };
                    if (allowed_chars.find(*m_ptr) == std::string_view::npos) {
                        std::cout << __FILE__ << ":" << __LINE__ << ": " << "DEBUG: unexpected char(" << get_pos().line << ":"
                            << get_pos().col << "): " << *m_ptr << "(" << int(*m_ptr) << ")" << std::endl;
                    }
                }
            }

        } while (*(++m_ptr));

        return XmlLexTokenType::EndOfFile;
    }



    inline void XmlLexer::skip_whitespace() {
        // TODO: Windows newlines
        while (*m_ptr && (*m_ptr == '\n' || *m_ptr == '\t' || *m_ptr == ' ')) {
            if (*m_ptr == '\n') {
                ++m_line;
                m_last_line_end = m_ptr;
            }
            ++m_ptr;
        }
        m_last_value = { m_ptr, 0 };

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
        case XmlLexTokenType::Quote:
            return os << "\"";
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
        }
    }
    void XmlLexer::remove_comment() {
        while (m_ptr + 2 < m_data.data() + m_data.length() && m_ptr[0] != '-' || m_ptr[1] != '-' || m_ptr[2] != '>') {
            ++m_ptr;
        }
        m_ptr += 3;
        if (m_ptr == m_data.data() + m_data.length()) {
            throw std::runtime_error{ "Unterminated comment" };
        }
    }
}

