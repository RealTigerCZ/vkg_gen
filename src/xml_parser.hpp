/**
 * @file xml_parser.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief
 * @date 30. 07. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

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
        Quote,                // "
        Identifier,           // tag names, attribute names
        Text,                 // text content
        XmlTagStart,          // <?xml
        XmlTagEnd,            // ?>
        EndOfFile
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

    class XmlLexer {

        sv m_last_value = {};

        const std::string& m_data;
        char const* m_ptr = m_data.data();
        char const* m_last_line_end = m_ptr - 1; // CHECK: Points to the end of the last line, could be out of bounds
        int m_line = 1;

        void skip_whitespace();
        XmlLexTokenType load_text(bool allow_text);

    public:
        XmlLexTokenType next(bool skip_whitespace, bool allow_text);
        sv get_value() const { return m_last_value; }
        XmlPosition get_pos() const { return { m_line, m_ptr - m_last_line_end }; }


        XmlLexer(const std::string& data) : m_data{ data } {};
    };

    class XmlParser {

    public:
        XmlDom parse(const std::string& path);

        XmlParser() = default;
        ~XmlParser() = default;
    };

} // namespace vkg_gen
