/**
 *@file main.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief
 * @date Created: 30. 07. 2025
 * @date Modified: 20. 09. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include <iostream>
#include "xml_parser.hpp"

static const char* FILE_PATH = "vk.xml";

int main() {
    vkg_gen::XmlParser parser;
    vkg_gen::XmlDom dom = parser.parse(FILE_PATH);
    vkg_gen::XmlLexer lexer(dom.file.data);
    bool skip_whitespace = true;
    bool allow_text = false;

    while (true) {
        auto token = lexer.next(skip_whitespace, allow_text);
        auto pos = lexer.get_pos();
        std::cout << "Pos: " << FILE_PATH << ":" << pos.line << ":" << pos.col << "\n";
        std::cout << "Token: " << token << std::endl;
        if (token == vkg_gen::XmlLexTokenType::Identifier || token == vkg_gen::XmlLexTokenType::Text) {
            std::cout << "Value: '" << lexer.get_value() << "'" << std::endl;
            if (lexer.get_value().size() == 0)
                break;
        }

        if (token == vkg_gen::XmlLexTokenType::EndOfFile)
            break;
        if (token == vkg_gen::XmlLexTokenType::LessThan || token == vkg_gen::XmlLexTokenType::LessThanSlash) {
            skip_whitespace = false;
            allow_text = false;
        } else if (token == vkg_gen::XmlLexTokenType::GreaterThan || token == vkg_gen::XmlLexTokenType::SlashGreaterThan) {
            skip_whitespace = true;
            allow_text = true;
        } else if (token == vkg_gen::XmlLexTokenType::Quote) {
            allow_text = !allow_text;
        } else if (token == vkg_gen::XmlLexTokenType::Identifier) {
            skip_whitespace = true;
        }

    }

    return 0;
}
