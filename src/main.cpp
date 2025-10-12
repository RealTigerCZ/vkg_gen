/**
 *@file main.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 30. 07. 2025
 * @date Modified: 12. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include <iostream>
#include "xml_parser.hpp"

static const char* FILE_PATH = "vk.xml";

int main() {
    vkg_gen::XmlParser parser;
    vkg_gen::XmlDom dom = parser.parse(FILE_PATH);
    vkg_gen::XmlLexer lexer(dom.data, FILE_PATH);

    using Expected = vkg_gen::XmlLexer::Expected;
    Expected next = Expected::Header;

    while (true) {
        vkg_gen::XmlLexTokenType token;
        try {
            token = lexer.next(next);
        }
        catch (const vkg_gen::LexerError& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        };

        auto pos = lexer.get_pos();
        std::cout << "Pos: " << FILE_PATH << ":" << pos.line << ":" << pos.col << "\n";
        std::cout << "Token: " << token << std::endl;
        if (token == vkg_gen::XmlLexTokenType::Identifier || token == vkg_gen::XmlLexTokenType::Text || token == vkg_gen::XmlLexTokenType::String) {
            std::cout << "Value: '" << lexer.get_value() << "'" << std::endl;
        }

        if (token == vkg_gen::XmlLexTokenType::EndOfFile)
            break;
        if (token == vkg_gen::XmlLexTokenType::LessThan || token == vkg_gen::XmlLexTokenType::LessThanSlash) {
            next = Expected::Tag;
        } else if (token == vkg_gen::XmlLexTokenType::GreaterThan || token == vkg_gen::XmlLexTokenType::SlashGreaterThan || token == vkg_gen::XmlLexTokenType::XmlTagEnd) {
            next = Expected::Text;
        } else if (next == Expected::Tag || next == Expected::Header) {
            next = Expected::Attribute;
        }
    }

    return 0;
}
