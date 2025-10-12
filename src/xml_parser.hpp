/**
 * @file xml_parser.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 30. 07. 2025
 * @date Modified: 12. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

#define UNREACHABLE() std::cerr << "Unreachable code reached at " __FILE__ ":" << __LINE__; std::abort();

#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <fstream>

#include "arena.hpp"

namespace vkg_gen {

    using sv = std::string_view;

    template <typename T>
    using vec = std::vector<T>;

    enum class XmlLexTokenType {
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
        EndOfFile,

    };

    /**
     * @brief Prints the XmlLexTokenType to the output stream
     */
    std::ostream& operator<<(std::ostream& os, XmlLexTokenType type);


    struct XmlHeader {
        sv version;
        sv encoding;
        sv standalone;
    };

    struct XmlAttr {
        sv name; // TODO: replace with interned string
        sv value;
        bool value_is_interned; // TODO: implement :)
    };

    struct XmlNode;


    struct XmlElement {
        sv tag; // TODO: replace with interned string
        vec<XmlAttr> attrs;
        vec<XmlNode*> children;
    };

    struct XmlNode {
        using XmlText = sv;
        using Data = std::variant<XmlElement, XmlText>;

        Data data;
        XmlNode* parent = nullptr;

        bool isElement() const noexcept { return std::holds_alternative<XmlElement>(data); }
        bool isText() const noexcept { return std::holds_alternative<XmlText>(data); }

        XmlElement& asElement() noexcept { return std::get<XmlElement>(data); }
        const XmlElement& asElement() const noexcept { return std::get<XmlElement>(data); }

        XmlText& asText() noexcept { return std::get<XmlText>(data); }
        const XmlText& asText() const noexcept { return std::get<XmlText>(data); }
    };


    struct XmlDom {
        std::string data; // Holds the copy of the file data // TODO: replace with string view?
        XmlNode* root;
        XmlHeader* header;
        Arena arena; // All nodes are allocated in the arena
    };

    struct XmlPosition {
        int line;
        int col;
    };


    class LexerError;

    class XmlLexer {
    public:
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

        friend LexerError;

        static sv next_utf8_char(const char* p) {
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

        XmlLexTokenType load_text();
        XmlLexTokenType load_identifier();
        XmlLexTokenType load_string();

        void skip_whitespace();
        void handle_new_line();
        void remove_comment();
        void escape_entity();

    public:
        XmlLexTokenType next(Expected expected);

        Slice get_last_value() const { return m_last_value; }
        // TODO: returned SV could be invalidated by lexer::next call
        sv get_value() const { return { m_buffer.data() + m_last_value.start, m_last_value.size }; }
        XmlPosition get_pos() const { return { m_line, m_ptr - m_last_line_end }; }


        XmlLexer(const std::string& data, const char* path) : m_data(data), file_path(path) {};


    };

    class XmlParser {

    public:
        XmlDom parse(const std::string& path);

        XmlParser() = default;
        ~XmlParser() = default;
    };


    class LexerError : public std::runtime_error {
    public:
        LexerError(const XmlLexer& lexer, const std::string& msg, int len = 1);
    };

} // namespace vkg_gen
