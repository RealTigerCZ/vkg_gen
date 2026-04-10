/**
 * @file xml.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Helper implementation of xml data structures
 *
 * @date Created: 13. 10. 2025
 * @date Modified: 10. 04. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */
#include "xml.hpp"
#include <sstream>
#include <algorithm>
#include <ranges>
#include <queue>
#include <functional>

#include "../debug_macros.h"

namespace vkg_gen::xml {
    static void free_node(Node* node) {
        if (node->isElement()) {
            auto& elem = node->asElement();
            for (Node* child : elem.children) {
                free_node(child);
            }
            elem.~Element();
        }
    }

    Dom::~Dom() {
        free_node(root);
    }

    Error::Error(const std::string& data, const Position& pos, const char* action, const char* file_path, const std::string& msg,
        const int err_start, const int err_len, const int line_start, bool backwards) : std::runtime_error([&] {
            std::stringstream ss;
            ss << "Error occured when " << action << " at " << file_path << ':' << pos.line << ':' << pos.col << '\n' << msg << '\n';
            if (err_len == 0)
                return ss.str();

            sv line{ &data[line_start], data.data() + data.size() };
            size_t new_size = line.find_first_of('\n');
            if (new_size != sv::npos)
                line = line.substr(0, new_size);

            int error_loc = err_start - line_start - backwards * err_len;
            ss << pos.line << " | " << line << '\n';
            ss << pos.line << " | " << std::string(error_loc, ' ') << std::string(err_len, '^') << "~~~here" << std::endl;
            return ss.str();
            }()) {};


    vec<Node*> Dom::getChildren(Node* node, bool recursive) const noexcept {
        if (node == nullptr) node = root;

        if (!node->isElement()) return {};
        auto& elem = node->asElement();
        if (!recursive) return elem.children;

        // FIXME: do we need recursive search? and if yes, how should it work
        // 1: search every node below and return all matches
        // 2: search every branch and return first matches in them
        std::vector<Node*> result;
        std::queue<Node*> queue;
        queue.push(node);

        while (!queue.empty()) {
            Node* node = queue.front();
            queue.pop();
            result.push_back(node);
            if (node->isElement()) {
                auto& elem = node->asElement();
                for (Node* child : elem.children) {
                    queue.push(child);
                }
            }
        }
        return result;

    };

    vec<Node*> Dom::getChildrenByTag(sv tag, Node* node, bool recursive) const noexcept {
        if (node == nullptr) node = root;

        if (!node->isElement()) return {};
        if (tag.empty()) return getChildren(node, recursive);

        auto& elem = node->asElement();
        vec<Node*> result;
        if (!recursive) {
            for (auto& child : elem.children)
                if (child->isElement() && child->asElement().tag == tag)
                    result.push_back(child);
            return result;
        }

        // FIXME: do we need recursive search? and if yes, how should it work
        // 1: search every node below and return all matches
        // 2: search every branch and return first matches in them

    };

    vec<Node*> Dom::getChidlrenByAttrName(sv name, Node* node, bool recursive) const noexcept {
        if (node == nullptr) node = root;
        if (!node->isElement()) return {};
        if (name.empty()) return getChildren(node, recursive);

        auto& elem = node->asElement();
        vec<Node*> result;

        if (!recursive) {
            for (Node* child : elem.children) {
                if (!child->isElement())
                    continue;
                auto& attrs = child->asElement().attrs;
                if (std::ranges::find(attrs, name, &Attribute::name) != attrs.end())
                    result.push_back(child);
            }
            return result;
        }

        // FIXME: do we need recursive search? and if yes, how should it work
        // 1: search every node below and return all matches
        // 2: search every branch and return first matches in them
    }

    vec<Node*> Dom::getChidlrenByAttrValue(std::optional<sv> value, Node* node, bool recursive) const noexcept {
        if (node == nullptr) node = root;
        if (!node->isElement()) return {};
        if (!value.has_value()) return getChildren(node, recursive);

        auto& elem = node->asElement();
        auto& val = value.value();
        vec<Node*> result;

        if (!recursive) {
            for (Node* child : elem.children) {
                if (!child->isElement())
                    continue;
                auto& attrs = child->asElement().attrs;
                if (std::ranges::find(attrs, val, &Attribute::value) != attrs.end())
                    result.push_back(child);
            }
            return result;
        }

        // FIXME: do we need recursive search? and if yes, how should it work
        // 1: search every node below and return all matches
        // 2: search every branch and return first matches in them
    }


    vec<Node*> Dom::getChildrenByFilter(const Filter& filter, Node* node, bool recursive) const noexcept {
        if (node == nullptr) node = root;
        if (!node->isElement()) return {};
        if (filter.tag.empty() && filter.attr_name.empty() && !filter.attr_value.has_value()) return getChildren(node, recursive);


        constexpr auto only_attr_value = [](const Element& elem, const Filter& filter) {
            return std::ranges::find(elem.attrs, filter.attr_value.value(), &Attribute::value) != elem.attrs.end();
            };

        constexpr auto only_attr_name = [](const Element& elem, const Filter& filter) {
            return std::ranges::find(elem.attrs, filter.attr_name, &Attribute::name) != elem.attrs.end();
            };

        constexpr auto both_atrrs = [](const Element& elem, const Filter& filter) {
            for (auto& attr : elem.attrs) {
                if (attr.name == filter.attr_name && attr.value == filter.attr_value.value())
                    return true;
            }
            return false;
            };


        const auto find_attrs = !filter.attr_value.has_value() && !filter.attr_name.empty() ? [](const Element& elem, const Filter& filter) {return true;} :
            filter.attr_value.has_value() ? filter.attr_name.empty() ? only_attr_value : both_atrrs : only_attr_name;

        auto& elem = node->asElement();
        vec<Node*> result;

        for (Node* child : elem.children) {
            if (!child->isElement())
                continue;
            auto& elem = child->asElement();
            if (!filter.tag.empty() && elem.tag != filter.tag)
                continue;

            if (!find_attrs(elem, filter))
                continue;

            result.push_back(child);
        }

        return result;
    };

    vec<Node*> Dom::getChildrenByChildrenFilter(const Filter& filter, Node* node) const noexcept {
        NOT_IMPLEMENTED();
        return {};
    };





    auto Dom::filterByAttr(sv name, sv value, Node* node, bool recursive) const noexcept {

    };

    auto Dom::filterByFilter(const Filter& filter, Node* node, bool recursive) const noexcept {

    };

    auto Dom::filterByTag(auto nodes, sv tag) const noexcept {

    };

    auto Dom::filterByAttr(auto nodes, sv name, sv value) const noexcept {

    };

    auto Dom::filterByFilter(auto nodes, const Filter& filter) const noexcept {

    };

    auto Dom::filterByChildrenFilter(auto nodes, const Filter& filter) const noexcept {

    };




}
