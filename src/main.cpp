/**
 *@file main.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 30. 07. 2025
 * @date Modified: 1. 11. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include <iostream>
#include "xml/parser.hpp"
#include "xml/lexer.hpp"
#include <set>
#include <unordered_map>

static const char* FILE_PATH = "vk.xml";


void debug_print_node(const vkg_gen::xml::Node& node, int indent = 0, int max_indent = 0) {
    if (max_indent > 0 && indent > max_indent)
        return;

    if (node.isElement()) {
        auto& elem = node.asElement();
        std::cout << std::string(indent, ' ') << " - Tag: " << elem.tag << '\n';
        if (elem.attrs.size() > 0) {
            std::cout << std::string(indent, ' ') << " - Attrs: ";
            for (auto& attr : elem.attrs) {
                std::cout << attr.name << "=" << attr.value << ", ";
            }
            std::cout << "\n";
        }

        if (elem.children.size() > 0) {
            std::cout << std::string(indent, ' ') << " - Children: " << elem.children.size() << "\n";

            for (auto& child : elem.children) {
                debug_print_node(*child, indent + 2, max_indent);
            }
        }
    }
}

std::string node_signature(const vkg_gen::xml::Node& node) {
    if (!node.isElement()) return "TEXT";
    auto& e = node.asElement();

    std::string sig = std::string(e.tag);
    if (e.children.size() == 0) return sig;

    sig += "(";
    for (auto& child : e.children) {
        if (child != e.children[0]) // pointer equality
            sig += ",";
        if (child->isElement())
            sig += std::string(child->asElement().tag);
        else
            sig += "TEXT";
    }
    sig += ")";
    return sig;
}


void debug_print_node_short(const vkg_gen::xml::Node& node, int indent = 0) {
    if (!node.isElement()) return;
    const auto& elem = node.asElement();

    std::cout << std::string(indent, ' ') << "- <" << elem.tag << ">\n";

    // Count identical simple children and structural patterns
    struct Summary {
        std::string tag;
        std::string sig;
        int count = 0;
        const vkg_gen::xml::Node* sample = nullptr;
    };
    std::unordered_map<std::string, Summary> patterns;

    for (auto& child : elem.children) {
        if (!child->isElement()) continue;
        const auto& ch = child->asElement();

        std::string sig = node_signature(*child);
        auto& summary = patterns[sig];
        summary.tag = std::string(ch.tag);
        summary.sig = std::move(sig);
        summary.count++;
        if (!summary.sample) summary.sample = child;
    }

    // Print summaries
    for (auto& [_, summary] : patterns) {
        if (summary.count > 1) {
            std::cout << std::string(indent + 2, ' ')
                << "- <" << summary.sig << "> x " << summary.count << '\n';
        } else if (summary.sample) {
            // Only one of this structure: recurse into it
            debug_print_node_short(*summary.sample, indent + 2);
        }
    }
}

void debug_print(const vkg_gen::xml::Dom& dom) {
    std::cout << "File: " << FILE_PATH << '\n';
    std::cout << "Header: " << '\n';
    std::cout << " - version: " << dom.header->version << '\n';
    std::cout << " - encoding: " << dom.header->encoding << '\n';
    std::cout << " - standalone: " << dom.header->standalone << '\n';
    std::cout << "Root: " << '\n';
    debug_print_node_short(*dom.root);
}

void free_node(vkg_gen::xml::Node* node) {
    if (node->isElement()) {
        auto& elem = node->asElement();
        for (auto& child : elem.children) {
            free_node(child);
        }
        node->asElement().~Element();
    }
}

void free_dom(vkg_gen::xml::Dom& dom) {
    free_node(dom.root);
}

int main() {
    namespace xml = vkg_gen::xml;

    xml::Parser parser;
    try {
        auto dom = parser.parse(FILE_PATH);
        free_dom(dom);
    }
    catch (const xml::Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }


#if 0
    xml::Lexer lexer(dom.data, FILE_PATH);

    using Expected = xml::Lexer::Expected;
    Expected next = Expected::Header;

    while (true) {
        TokenType token;
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
        if (token == TokenType::Identifier || token == TokenType::Text || token == TokenType::String) {
            std::cout << "Value: '" << lexer.get_value() << "'" << std::endl;
        }

        if (token == TokenType::EndOfFile)
            break;
        if (token == TokenType::LessThan || token == TokenType::LessThanSlash) {
            next = Expected::Tag;
        } else if (token == TokenType::GreaterThan || token == TokenType::SlashGreaterThan || token == TokenType::XmlTagEnd) {
            next = Expected::Text;
        } else if (next == Expected::Tag || next == Expected::Header) {
            next = Expected::Attribute;
        }
    }
#endif
    return 0;
}
