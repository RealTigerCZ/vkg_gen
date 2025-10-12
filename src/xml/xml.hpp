/**
 *@file xml.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Definitions of xml data structures
 *
 * @date Created: 12. 10. 2025
 * @date Modified: 12. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

#define UNREACHABLE() std::cerr << "Unreachable code reached at " __FILE__ ":" << __LINE__; std::abort();

#include "../arena.hpp"
#include <variant>

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

    struct Dom {
        std::string data; // Holds the copy of the file data // TODO: replace with string view?
        Node* root;
        Header* header;
        Arena arena; // All nodes are allocated in the arena
    };

    struct Position {
        int line;
        int col;
    };
}
