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
#include "xml/parser.hpp"
#include "xml/lexer.hpp"

static const char* FILE_PATH = "vk.xml";

int main() {
    namespace xml = vkg_gen::xml;

    xml::Parser parser;
    xml::Dom dom = parser.parse(FILE_PATH);
    xml::Lexer lexer(dom.data, FILE_PATH);

    using Expected = xml::Lexer::Expected;
    Expected next = Expected::Header;

    while (true) {
        xml::LexTokenType token;
        try {
            token = lexer.next(next);
        }
        catch (const xml::LexerError& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        };

        auto pos = lexer.get_pos();
        std::cout << "Pos: " << FILE_PATH << ":" << pos.line << ":" << pos.col << "\n";
        std::cout << "Token: " << token << std::endl;
        if (token == xml::LexTokenType::Identifier || token == xml::LexTokenType::Text || token == xml::LexTokenType::String) {
            std::cout << "Value: '" << lexer.get_value() << "'" << std::endl;
        }

        if (token == xml::LexTokenType::EndOfFile)
            break;
        if (token == xml::LexTokenType::LessThan || token == xml::LexTokenType::LessThanSlash) {
            next = Expected::Tag;
        } else if (token == xml::LexTokenType::GreaterThan || token == xml::LexTokenType::SlashGreaterThan || token == xml::LexTokenType::XmlTagEnd) {
            next = Expected::Text;
        } else if (next == Expected::Tag || next == Expected::Header) {
            next = Expected::Attribute;
        }
    }

    return 0;
}
