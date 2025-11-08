/**
 * @file parser.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 * @date Created: 12. 10. 2025
 * @date Modified: 1. 11. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#include "parser.hpp"
#include "lexer.hpp"
#include "lex_token_formater.hpp"

#include <fstream>
#include <cstring>
#include <stack>

#include <assert.h>
#include <iostream>
#include <sstream>


#define DEBUG_HERE(...) std::cout << __FILE__ << ":" << __LINE__ << ": " << __VA_ARGS__ << std::endl;
#define HERE() std::cout << __FILE__ << ":" << __LINE__ << std::endl;

#define EXPECT_FMT(exp, ...) std::format("Expected " exp " but got '{}'!", __VA_ARGS__)

namespace vkg_gen::xml {
    Dom Parser::parse(const std::string& path) {
        std::ifstream file{ path, std::ios::in };
        if (!file.is_open()) {
            throw std::runtime_error{ "Failed to open file: '" + path + "' because: '" + std::strerror(errno) + "'" };
        }

        Dom dom(std::string{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() });

        Lexer lexer{ dom.data, path.c_str() };

        parse(dom, lexer);

        return dom;
    };

    void Parser::parse(Dom& dom, Lexer& lexer) {
        using Expected = Lexer::Expected;
        using TokenType = Lexer::TokenType;


        load_header(lexer, dom);

        TokenType token = lexer.next(Expected::Text);
        if (token == TokenType::Text) {
            std::cout << "TODO: skipping text before root tag: '" << lexer.get_value() << '\'' << std::endl;
            token = lexer.next(Expected::Text);
        }

        if (token != TokenType::LessThan)
            throw ParserError{ lexer,  EXPECT_FMT("'<'", token) };


        token = lexer.next(Expected::Tag);
        if (token != TokenType::Identifier)
            throw ParserError{ lexer, std::format("Did not find root tag name. Expected identifier but got '{}'", token) };

        dom.root = dom.arena.make<Node>();
        auto& root = dom.root->asElement();
        root.tag = lexer.get_and_save_value(dom.arena); // FIXME: replace with interned string

        std::stack<Node*> stack({ dom.root });
        if (load_attr(lexer, root.attrs, dom)) {
            stack.pop();
        }

        Expected next = Expected::Text;

        while (true) {

            assert(next == Expected::Text);
            TokenType token = lexer.next(next);
            if (stack.empty()) {
                if (token == TokenType::Text) {
                    std::cout << "TODO: skipping text before root tag: '" << lexer.get_value() << '\'' << std::endl;
                    token = lexer.next(Expected::Text);
                }

                if (token == TokenType::EndOfFile)
                    return;

                throw ParserError{ lexer, "Unexpected token after root element." };
            }

            switch (token) {
            case TokenType::LessThan: {
                    token = lexer.next(Expected::Tag);
                    if (token != TokenType::Identifier)
                        throw ParserError{ lexer, EXPECT_FMT("tag name (ID)", token), };

                    Node* element = dom.arena.make<Node>();
                    element->asElement().tag = lexer.get_and_save_value(dom.arena);
                    stack.top()->asElement().children.push_back(element);
                    element->parent = stack.top();
                    if (!load_attr(lexer, element->asElement().attrs, dom))
                        stack.push(element);

                    next = Expected::Text;
                    break;

                }

            case TokenType::LessThanSlash:
                token = lexer.next(Expected::Tag);
                if (token != TokenType::Identifier)
                    throw ParserError{ lexer, EXPECT_FMT("tag name (ID)", token) };

                if (lexer.get_value() != stack.top()->asElement().tag)

                    throw ParserError{ lexer, std::format("Can't close tag '{}' with '{}'", stack.top()->asElement().tag, lexer.get_value()) }; // TODO: point to the opening tag

                stack.pop();
                token = lexer.next(Expected::Attribute);
                if (token != TokenType::GreaterThan)
                    throw ParserError{ lexer, EXPECT_FMT("end of tag '>'", token) + " Did you try to add attributes to closing tag?" };
                break;

            case TokenType::Text: {
                    Node* text = dom.arena.make<Node>(Node::Text{ lexer.get_and_save_value(dom.arena) }); // FIXME: replace with interned string
                    stack.top()->asElement().children.push_back(text);
                    break;
                }

            case TokenType::XmlTagStart:
                throw ParserError{ lexer, "Unexpected '<?xml'." };

            case TokenType::EndOfFile:
                assert(!stack.empty()); // Empty stack handled above
                {
                    std::stringstream ss;
                    ss << "Unexpected end of file. These elements are still open: ";
                    while (!stack.empty()) {
                        ss << stack.top()->asElement().tag;
                        stack.pop();
                        if (!stack.empty())
                            ss << ", ";
                    }
                    throw ParserError{ lexer, ss.str(), 0 };
                }

            case TokenType::Identifier: // unreachable from Expected::Text, only in Expected::Tag
            case TokenType::XmlTagEnd: // unreachable from Expected::Text, only in Expected::Header
            case TokenType::GreaterThan:// unreachable from Expected::Text, only in Expected::Attribute
            case TokenType::SlashGreaterThan: // unreachable from Expected::Text, only in Expected::Attribute
            case TokenType::Equals: // unreachable from Expected::Text, only in Expected::Attribute
            case TokenType::String: // unreachable from Expected::Text, only in Expected::Attribute
                UNREACHABLE();
            }
        }
    };

    void Parser::load_header(Lexer& lexer, Dom& dom) {
        using Expected = Lexer::Expected;
        using TokenType = Lexer::TokenType;

        TokenType token = lexer.next(Expected::Header);
        if (token != TokenType::XmlTagStart)
            throw ParserError{ lexer, EXPECT_FMT("'<?xml' tag", token) };

        dom.header = dom.arena.make<Header>();

        bool has_version = false;
        bool has_encoding = false;
        bool has_standalone = false;


        while (true) {
            token = lexer.next(Expected::Attribute);
            if (token == TokenType::XmlTagEnd)
                return;
            if (token != TokenType::Identifier)
                throw ParserError{ lexer, EXPECT_FMT("attribute name (ID) or '?>'", token) };

            int start = lexer.get_token_start();
            sv attr_name = lexer.get_and_save_value(dom.arena);
            if (attr_name != "version" && attr_name != "encoding" && attr_name != "standalone")
                throw ParserError{ lexer, EXPECT_FMT("'version', 'encoding' or 'standalone' attribute name in header", attr_name) };

            token = lexer.next(Expected::Attribute);
            if (token != TokenType::Equals)
                throw ParserError{ lexer, EXPECT_FMT("'=' after attribute name", token) };

            token = lexer.next(Expected::Attribute);
            if (token != TokenType::String)
                throw ParserError{ lexer, EXPECT_FMT("attribute value (STR)", token) };

            int end = lexer.get_token_end();
            if (attr_name == "version") {
                if (has_version)
                    throw ParserError{ lexer, "Duplicate 'version' attribute.", end - start };
                dom.header->version = lexer.get_and_save_value(dom.arena);
                has_version = true;
            } else if (attr_name == "encoding") {
                if (has_encoding)
                    throw ParserError{ lexer, "Duplicate 'encoding' attribute.",  end - start };
                dom.header->encoding = lexer.get_and_save_value(dom.arena);
                has_encoding = true;
            } else {
                if (has_standalone)
                    throw ParserError{ lexer, "Duplicate 'standalone' attribute.", end - start };
                dom.header->standalone = lexer.get_and_save_value(dom.arena);
                has_standalone = true;
            }
        }

    };

    bool Parser::load_attr(Lexer& lexer, vec<Attribute>& attrs, Dom& dom) {
        using Expected = Lexer::Expected;
        using TokenType = Lexer::TokenType;


        sv name; // TODO: replace with interned string
        sv value;
        bool value_is_interned = false; //FIXME:

        while (true) {
            TokenType token = lexer.next(Expected::Attribute);
            if (token == TokenType::GreaterThan)
                return false;
            if (token == TokenType::SlashGreaterThan)
                return true;

            if (token != TokenType::Identifier)
                throw ParserError{ lexer, EXPECT_FMT("attribute name or end of the tag ('>' or '/>')", token) };

            name = lexer.get_and_save_value(dom.arena);

            auto dupl = std::find_if(attrs.begin(), attrs.end(), [&name](const Attribute& a) { return a.name == name; });
            if (dupl != attrs.end())
                throw ParserError{ lexer, std::format("Found duplicate attribute '{}' in tag.", name) };

            token = lexer.next(Expected::Attribute);
            if (token != TokenType::Equals)
                throw ParserError{ lexer, EXPECT_FMT("'=' after attribute name", token) };

            token = lexer.next(Expected::Attribute);
            if (token != TokenType::String)
                throw ParserError{ lexer, EXPECT_FMT("attribute value (STR)", token) };

            value = lexer.get_and_save_value(dom.arena);
            value_is_interned = false; // TODO: replace with interned string

            attrs.emplace_back(Attribute{ name, value, value_is_interned });
        }
    }

    ParserError::ParserError(const Lexer& lexer, const std::string& msg, int len) :
        Error(lexer.get_err_loc(), msg, "parsing", len == USE_TOKEN_LEN ? lexer.get_value().size() : len, true) {};

} // namespace vkg_gen::xml
