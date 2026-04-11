/**
 *@file xml.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Definitions of xml data structures
 *
 * @date Created: 12. 10. 2025
 * @date Modified: 11. 04. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

#include "../debug_macros.h"
#include "../arena.hpp"
#include <variant>
#include <ranges>
#include <optional>
#include <stdexcept>

namespace vkg_gen::xml {
    using sv = std::string_view;

    template <typename T>
    using vec = std::vector<T>;

    struct Header {
        sv version;
        sv encoding;
        sv standalone;
    };

    struct Attribute {
        sv name; // TODO: replace with interned string
        sv value;
        bool value_is_interned; // TODO: implement :)
    };

    struct Node;


    struct Element {
        sv tag; // TODO: replace with interned string
        vec<Attribute> attrs;
        vec<Node*> children;

        sv get_attr_value(sv name) const {
            auto it = std::ranges::find(attrs, name, &Attribute::name);
            if (it == attrs.end())
                return {};
            return it->value;
        }
    };

    struct Node {
        using Text = sv;
        using Data = std::variant<Element, Text>;

        Data data;
        Node* parent = nullptr;

        bool isElement() const noexcept { return std::holds_alternative<Element>(data); }
        bool isText() const noexcept { return std::holds_alternative<Text>(data); }

        Element& asElement() noexcept { return std::get<Element>(data); }
        const Element& asElement() const noexcept { return std::get<Element>(data); }

        Text& asText() noexcept { return std::get<Text>(data); }
        const Text& asText() const noexcept { return std::get<Text>(data); }
    };

    class Dom {
    public:
        struct Filter {
            sv tag;
            sv attr_name;
            std::optional<sv> attr_value;
        };

        std::string data; // Holds the copy of the file data // TODO: do we need to replace it with string view?
        Node* root;
        Header* header;
        Arena arena; // All nodes are allocated in the arena

        Dom(std::string&& data) : data(std::move(data)) {};
        ~Dom();

        vec<Node*> getChildren(Node* node = nullptr, bool recursive = false) const noexcept;
        vec<Node*> getChildrenByTag(sv tag, Node* node = nullptr, bool recursive = false) const noexcept;
        vec<Node*> getChidlrenByAttrName(sv name, Node* node = nullptr, bool recursive = false) const noexcept;
        vec<Node*> getChidlrenByAttrValue(std::optional<sv> value, Node* node = nullptr, bool recursive = false) const noexcept;

        vec<Node*> getChildrenByFilter(const Filter& filter, Node* node = nullptr, bool recursive = false) const noexcept;
        vec<Node*> getChildrenByChildrenFilter(const Filter& filter, Node* node = nullptr) const noexcept;

        auto children(Node* node) const noexcept {
            //if (!node->isElement()) return {};
            auto& ch = node->asElement().children;
            return std::views::all(ch)
                | std::views::filter([](Node* n) { return n->isElement(); });
        }

        //        auto filterByTag(sv tag, Node* node = nullptr, bool recursive = false) const noexcept;
        auto filterByTag(sv tag, Node* node) const noexcept {
            if (node == nullptr) node = root;

            return children(node)
                | std::views::filter([&tag](Node* n) { return n->asElement().tag == tag; });

        };
        auto filterByAttr(sv name, sv value, Node* node = nullptr, bool recursive = false) const noexcept;
        auto filterByFilter(const Filter& filter, Node* node = nullptr, bool recursive = false) const noexcept;
        auto filterByTag(auto nodes, sv tag) const noexcept;
        auto filterByAttr(auto nodes, sv name, sv value) const noexcept;
        auto filterByFilter(auto nodes, const Filter& filter) const noexcept;
        auto filterByChildrenFilter(auto nodes, const Filter& filter) const noexcept;

        // No copying
        Dom(const Dom&) = delete;
        Dom& operator=(const Dom&) = delete;

        // Allow moves
        Dom(Dom&&) noexcept = default;
        Dom& operator=(Dom&&) noexcept = default;
    };

    struct Position {
        int line;
        int col;
    };

    struct ErrorLoc {
        const std::string& data;
        const Position pos;
        const char* file_path;
        const int err_start;
        const int line_start;
    };

    class Error : public std::runtime_error {
    public:
        Error(const std::string& data, const Position& pos, const char* action, const char* file_path,
            const std::string& msg, const int err_start, const int err_len, const int line_start, bool forward);

        Error(const ErrorLoc& e, const std::string& msg, const char* action, const int len, bool forward) :
            Error(e.data, e.pos, action, e.file_path, msg, e.err_start, len, e.line_start, forward) {};
    };

}
