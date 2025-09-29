/**
 * @file xml_parser.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 30. 07. 2025
 * @date Modified: 29. 09. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

#define UNREACHABLE() std::cerr << "Unreachable code reached at " __FILE__ ":" << __LINE__; std::abort();

#include <string>
#include <string_view>
#include <vector>

#include <fstream>

namespace vkg_gen {

    using sv = std::string_view;

    template <typename T>
    using vec = std::vector<T>;

    struct XmlAttr {
        sv name;
        sv value;
    };

    struct XmlItem { // FIXME: what about this: <tag1> Hello <tag2> World </tag2> </tag1>
        sv tag;
        sv value;
        vec<XmlAttr> attrs;
        vec<XmlItem> children;
    };

    struct XmlHeader {
        sv version;
        sv encoding;
    };

    struct XmlFile {
        const std::string data; // Holds the copy of xml data, **must live at least as long as XmlDom's items**
        const std::string path; // Path to xml file
    };

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

    struct XmlDom {
        XmlHeader header;
        XmlItem root;
        XmlFile file;
    };

    struct XmlPosition {
        int line;
        int col;
    };

    struct Slice {
        int start;
        int size;
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
