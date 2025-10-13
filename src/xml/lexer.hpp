/**
 * @file lexer.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Context-sensitive / stateful lexer (tokenizer) for subset of xml (mainly used for tokenizing vk.xml).
 *        It's used by the parser.
 *
 * @date Created: 30. 07. 2025
 * @date Modified: 13. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

#include "xml.hpp"

#include <iostream>

namespace vkg_gen::xml {
    class LexerError;

    class Lexer {
    public:
        enum class TokenType {
            LessThan,             // <
            GreaterThan,          // >
            SlashGreaterThan,     // />
            LessThanSlash,        // </
            Equals,               // =
            String,               // "..." or '...'
            Identifier,           // tag names, attribute names
            Text,                 // text content between tags
            XmlTagStart,          // <?xml
            XmlTagEnd,            // ?>
            EndOfFile
        };


        enum class Expected {
            Header,
            Text,
            Tag,
            Attribute
        };

        struct Slice {
            int start;
            int size;
        };

        static sv next_utf8_char(const char* p) noexcept {
            unsigned char c = *p;
            size_t len = 1;
            if (c >= 0xF0) len = 4;
            else if (c >= 0xE0) len = 3;
            else if (c >= 0xC0) len = 2;
            return sv(p, len);
        };

    private:
        std::vector<char> m_buffer;
        Slice m_last_value;

        const char* file_path;
        const std::string& m_data;
        char const* m_ptr = m_data.data();
        char const* m_last_line_end = m_ptr - 1; // CHECK: Points to the end of the last line, could be out of bounds
        int m_line = 1;

        TokenType load_text();
        TokenType load_identifier();
        TokenType load_string();

        void skip_whitespace();
        void handle_new_line();
        void remove_comment();
        void escape_entity();

        TokenType _next(Expected expected);
    public:
        TokenType next(Expected expected);

        Slice get_last_value() const noexcept { return m_last_value; }

        // TODO: returned SV could be invalidated by lexer::next call
        sv get_value() const noexcept { return { m_buffer.data() + m_last_value.start, static_cast<size_t>(m_last_value.size) }; }
        Position get_pos() const noexcept { return { m_line, static_cast<int>(m_ptr - m_last_line_end) }; }

        ErrorLoc get_err_loc() const noexcept {
            return { m_data, get_pos(), file_path, static_cast<int>(m_ptr - m_data.begin().base()),
                static_cast<int>(m_last_line_end + 1 - m_data.begin().base()) };
        }

        Lexer(const std::string& data, const char* path) : file_path(path), m_data(data) { m_buffer.reserve(1024 * 1024 * 4) /* FIXME: proper data handling ?*/; };
    };

    /**
     * @brief Prints the Lexer::TokenType to the output stream
     */
    std::ostream& operator<<(std::ostream& os, Lexer::TokenType type);



    class LexerError : public Error {
    public:
        LexerError(const Lexer& lexer, const std::string& msg, int len = 1);
    };


    constexpr sv token_to_string(Lexer::TokenType type) noexcept;
} // namespace vkg_gen::xml

