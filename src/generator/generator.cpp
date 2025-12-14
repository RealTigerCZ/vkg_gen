
#include <concepts>
#include <string_view>
#include <ranges>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cassert>
#include "../xml/xml.hpp"
#include "../arena.hpp"
#include "generator.hpp"


namespace boilerplate {
    static const char* HANDLE_DEFINITION = ""
        "// FIXME: \n"
        "template<typename Type> class UniqueHandle;\n"

        "    // author: PCJohn (peciva at fit.vut.cz)\n"
        "    template<typename T>\n"
        "    class Handle {\n"
        "    protected:\n"
        "        T _handle;\n"
        "    public:\n"
        "        using HandleType = T;\n"
        "\n"
        "        Handle() noexcept {}\n"
        "        Handle(std::nullptr_t) noexcept : _handle(nullptr) {}\n"
        "        Handle(T nativeHandle) noexcept : _handle(nativeHandle) {}\n"
        "        Handle(const Handle& h) noexcept : _handle(h._handle) {}\n"
        "        Handle(const UniqueHandle<Handle<T>>& u) noexcept : _handle(u.get().handle()) {}\n"
        "        Handle& operator=(const Handle rhs) noexcept { _handle = rhs._handle; return *this; }\n"
        "        Handle& operator=(const UniqueHandle<T>& rhs) noexcept { _handle = rhs._handle; return *this; }\n"
        "        T handle() const noexcept { return _handle; }\n"
        "        explicit operator bool() const noexcept { return _handle != nullptr; }\n"
        "        bool operator==(const Handle rhs) const noexcept { return _handle == rhs._handle; }\n"
        "        bool operator!=(const Handle rhs) const noexcept { return _handle != rhs._handle; }\n"
        "    };\n";

    //FIXME:
    static const char* VIDEO_INCLUDES = ""
        "#include \"vk_video/vulkan_video_codec_av1std_decode.h\"\n"
        "#include \"vk_video/vulkan_video_codec_h265std_decode.h\"\n"
        "#include \"vk_video/vulkan_video_codec_av1std_encode.h\"\n"
        "#include \"vk_video/vulkan_video_codec_h265std_encode.h\"\n"
        "#include \"vk_video/vulkan_video_codec_av1std.h\"\n"
        "#include \"vk_video/vulkan_video_codec_h265std.h\"\n"
        "#include \"vk_video/vulkan_video_codec_h264std_decode.h\"\n"
        "#include \"vk_video/vulkan_video_codecs_common.h\"\n"
        "#include \"vk_video/vulkan_video_codec_h264std_encode.h\"\n"
        "#include \"vk_video/vulkan_video_codec_vp9std_decode.h\"\n"
        "#include \"vk_video/vulkan_video_codec_h264std.h\"\n"
        "#include \"vk_video/vulkan_video_codec_vp9std.h\"\n";
}

using namespace vkg_gen::xml;

template <typename Func>
concept NodePredicate = requires(const Func & f, const Node * n) {
    { f(n) } -> std::convertible_to<bool>;
};

template <NodePredicate L, NodePredicate R>
struct And {
    L lhs;
    R rhs;
    inline constexpr bool operator()(const Node* n) const noexcept {
        return lhs(n) && rhs(n);
    }
};

template <NodePredicate L, NodePredicate R>
struct Or {
    L lhs;
    R rhs;
    inline constexpr bool operator()(const Node* n) const noexcept {
        return lhs(n) || rhs(n);
    }
};

template <NodePredicate L, NodePredicate R>
inline constexpr auto operator&&(L lhs, R rhs) noexcept {
    return And<L, R>{lhs, rhs};
}

template <NodePredicate L, NodePredicate R>
inline constexpr auto operator||(L lhs, R rhs) noexcept {
    return Or<L, R>{lhs, rhs};
}

struct HasTag {
    std::string_view tag;
    inline constexpr bool operator()(const Element& e) const noexcept {
        return e.tag == tag;
    }
    inline constexpr bool operator()(const Node* n) const noexcept {
        return n->isElement() && n->asElement().tag == tag;
    }
};

struct HasAttr {
    std::string_view name;
    std::string_view value;

    inline constexpr bool operator()(const Element& e) const noexcept {
        return std::ranges::any_of(e.attrs, [&](const auto& a) {
            return a.name == name && a.value == value;
            });
    }

    inline constexpr bool operator()(const Node* n) const noexcept {
        if (!n->isElement()) return false;
        return (*this)(n->asElement());
    }

};

struct HasAttrName {
    std::string_view name;

    constexpr inline bool operator()(const Element& e) const noexcept {
        return std::ranges::contains(e.attrs, name, &Attribute::name);
    }
    constexpr inline bool operator()(const Node* n) const noexcept {
        if (!n->isElement()) return false;
        return (*this)(n->asElement());
    }
};

struct HasAttrValue {
    std::string_view value;
    constexpr inline bool operator()(const Element& e) const noexcept {
        return std::ranges::contains(e.attrs, value, &Attribute::value);
    }
    constexpr inline bool operator()(const Node* n) const noexcept {
        if (!n->isElement()) return false;
        return (*this)(n->asElement());
    }
};


inline constexpr auto has_tag(std::string_view t) noexcept { return HasTag{ t }; }
inline constexpr auto has_attr(std::string_view n, std::string_view v) noexcept { return HasAttr{ n, v }; }
inline constexpr auto has_attr_name(std::string_view n) noexcept { return HasAttrName{ n }; }
inline constexpr auto has_attr_value(std::string_view v) noexcept { return HasAttrValue{ v }; }


std::ostream& helper_test(std::ostream& os, const vkg_gen::xml::Node* node) {
    if (!node) return os << "(NULL)";
    if (!node->isElement())
        return os << "(TEXT: " << node->asText() << ")";

    auto& elem = node->asElement();
    os << "<" << elem.tag;
    for (auto& attr : elem.attrs) {
        os << " " << attr.name << "=\"" << attr.value << '"';
    }
    return os << ">";
}


bool bool_from_string(std::string_view s) {
    if (s == "true")
        return true;
    if (s == "false")
        return false;

    if (s.find(",") != std::string_view::npos) {
        std::cout << "FIXME: " << s << "not supported" << std::endl;
        return false;
    }

    throw std::runtime_error("Invalid boolean value: '" + std::string(s) + "'");
}

Member::ExternSync externsync_from_string(std::string_view s) {
    if (s == "true")
        return Member::ExternSync::True;
    if (s == "false")
        return Member::ExternSync::False;
    if (s == "maybe")
        return Member::ExternSync::Maybe;

    throw std::runtime_error("Invalid externsync value: '" + std::string(s) + "'");
}




Type::Category Type::category_from_string(std::string_view s) const {
    if (s == "bitmask") return Category::Bitmask;
    if (s == "basetype") return Category::Basetype;
    if (s == "define") return Category::Define;
    if (s == "enum") return Category::Enum;
    if (s == "handle") return Category::Handle;
    if (s == "funcpointer") return Category::Funcpointer;
    if (s == "struct") return Category::Struct;
    if (s == "include") return Category::Include;
    if (s == "union") return Category::Union;

    std::string err("Invalid category: '");
    err += s;
    err += "' {";
    for (auto c : s) {
        err += c;
        err += ", ";
    }
    err += '}';

    throw std::runtime_error(err);
}

void Type::parse_struct(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena) {
    for (Node* child : elem.children) {
        if (child->isText()) {
            std::cout << "- Skipping TEXT: " << child->asText() << std::endl;
            continue;
        }

        auto& ch = child->asElement();

        if (ch.tag == "comment")
            struct_->members.emplace_back(Member(ch, arena, Member::ParentType::Struct, true));
        else if (ch.tag == "member")
            struct_->members.emplace_back(Member(ch, arena, Member::ParentType::Struct));
        else {
            throw std::runtime_error{ "Unknown child element: " + std::string(ch.tag) };
        }
    }

};


void Type::parse_union(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena) {
    for (Node* child : elem.children) {
        if (child->isText()) {
            std::cout << "- Skipping TEXT: " << child->asText() << std::endl;
            continue;
        }

        auto& ch = child->asElement();

        if (ch.tag == "comment")
            union_->members.emplace_back(Member(ch, arena, Member::ParentType::Union, true));
        else if (ch.tag == "member")
            union_->members.emplace_back(Member(ch, arena, Member::ParentType::Union));
        else {
            throw std::runtime_error{ "Unknown child element: " + std::string(ch.tag) };
        }
    }
};

Type::Type(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena) : elem(elem) {
    Category specific_attr = Category::None;

    for (const Attribute& attr : elem.attrs) {
        if (attr.name == "category") {
            category = category_from_string(attr.value);

            if (specific_attr == Category::None) {
                specific_attr = category;
                switch (category) {
                case Category::Handle:
                    handle = arena.make<TypeHandle>();
                    break;
                case Category::Struct:
                    struct_ = arena.make<TypeStruct>();
                    break;
                case Category::Union:
                    union_ = arena.make<TypeUnion>();
                    break;
                default:
                    break;
                }
            } else if (specific_attr != category)
                throw std::runtime_error("Invalid combination of attributes and category on <type>");

        } else if (attr.name == "name")
            name = attr.value;

        else if (attr.name == "api")
            api = attr.value;

        else if (attr.name == "alias")
            alias = attr.value;

        else if (attr.name == "requires")
            requires_ = attr.value;

        else if (attr.name == "comment")
            comment = attr.value;

        else if (attr.name == "deprecated")
            deprecated = attr.value;

        else if (attr.name == "parent" || attr.name == "objtypeenum") {
            if (specific_attr == Category::None) {
                handle = arena.make<TypeHandle>();
            } else if (specific_attr != Category::Handle) {
                throw std::runtime_error("unexpected attribute on <type>");
            }

            if (attr.name == "parent")
                handle->parent = attr.value;
            else
                handle->objtypeenum = attr.value;
        } else if (attr.name == "returnedonly") {
            if (specific_attr == Category::None) {
                //FIXME:
                auto it = std::ranges::find(elem.attrs, "category", &Attribute::name);
                if (it == elem.attrs.end())
                    throw std::runtime_error("unexpected attribute on <type>");
                specific_attr = category_from_string(it->value);

                if (specific_attr == Category::Struct) {

                    struct_ = arena.make<TypeStruct>();
                    struct_->returned_only = bool_from_string(attr.value);
                } else if (specific_attr == Category::Union) {

                    union_ = arena.make<TypeUnion>();
                    union_->returned_only = bool_from_string(attr.value);
                } else {
                    throw std::runtime_error("unexpected attribute on <type>");
                }


            } else if (specific_attr == Category::Struct) {
                struct_->returned_only = bool_from_string(attr.value);
            } else if (specific_attr == Category::Union) {
                union_->returned_only = bool_from_string(attr.value);
            } else {
                throw std::runtime_error("unexpected attribute on <type>");
            }



        } else if (attr.name == "structextends" || attr.name == "allowduplicate") {
            if (specific_attr == Category::None) {
                struct_ = arena.make<TypeStruct>();
            } else if (specific_attr != Category::Struct && specific_attr != Category::Union) {
                throw std::runtime_error("unexpected attribute on <type>");
            }

            if (attr.name == "structextends")
                struct_->struct_extends = attr.value;
            else if (attr.name == "allowduplicate")
                struct_->allow_duplicate = bool_from_string(attr.value);
            else
                struct_->returned_only = bool_from_string(attr.value);
        } else if (attr.name == "bitvalues") {
            if (specific_attr != Category::None && specific_attr != Category::Bitmask) {
                throw std::runtime_error("unexpected attribute on <type>");
            }

            specific_attr = Category::Bitmask;
            bitvalues = attr.value;
        } else {
            throw std::runtime_error("unknown attribute on <type>: " + std::string(attr.name));
        }

    }

    if (specific_attr == Category::Handle && handle->objtypeenum.empty() && alias.empty())
        throw std::runtime_error("objtypeenum attribute is required on <type category=\"handle\"> if alias attribute is not present");


    if (specific_attr == Category::Struct)
        parse_struct(elem, arena);

    if (specific_attr == Category::Union)
        parse_union(elem, arena);

    if (name.empty()) {
        auto it = std::ranges::find_if(elem.children, has_tag("name"));
        if (it != elem.children.end()) {
            name = (*it)->asElement().children[0]->asText();
        } else {
            throw std::runtime_error("name attribute or <name> tag is required in <type>");
        }
    };
}

void _gen_arbitrary_C_code_in_type(const vkg_gen::xml::Element& elem, std::ofstream& file) {
    bool last_was_element = false;
    for (Node* child : elem.children) {
        if (child->isText()) {
            // FIXME:
            if (child->asText() != ";" && last_was_element)
                file << ' ';
            file << child->asText();
            last_was_element = false;
            continue;
        }

        if (last_was_element)
            file << ' ';

        auto& ch = child->asElement();
        if (ch.tag == "comment") {
            file << "/*" << ch.children[0]->asText() << "*/"; // TODO: check_comment
        } else if (ch.tag == "name") {
            // TODO: register it
            file << ch.children[0]->asText(); // TODO: check_name
        } else if (ch.tag == "type") {
            // TODO: find if defined or at least declared;
            file << ch.children[0]->asText(); // TODO: check_type
        }


        last_was_element = true;
    }
    file << '\n';
}


//FIXME: helper function
bool exclude_platforms(sv name) {
    static sv platforms[] = {
        "Xlib",
        "Xcb",
        "Wayland",
        "DirectFB",
        "Android",
        "Win32",
        "Vi",
        "Ios",
        "Macos",
        "Metal",
        "Fuchsia",
        "Ggp",
        "Sci",
        "Provisional",
        "Screen",
        "Ohos"
    };
    static sv platforms_caps[] = {
        "XLIB",
        "XCB",
        "WAYLAND",
        "DIRECTFB",
        "ANDROID",
        "WIN32",
        "VI",
        "IOS",
        "MACOS",
        "METAL",
        "FUCHSIA",
        "GGP",
        "SCI",
        "PROVISIONAL",
        "SCREEN",
        "OHOS"
    };

    if (std::ranges::any_of(platforms_caps, [&name](sv& plat) { return name.ends_with(plat); })) return true;
    if (std::ranges::any_of(platforms, [&name](sv& plat) {
        auto pos = name.find(plat);
        if (pos == std::string::npos)
            return false;
        auto next_index = pos + plat.size();
        if (next_index >= name.size()) return false;
        return (name[next_index] >= 'A' && name[next_index] <= 'Z');
        })) return true;

    return false;
}



void generate_type(const vkg_gen::xml::Element& type, vkg_gen::Arena& arena, std::ofstream& file) {
    Type t(type, arena);

    if (!t.comment.empty()) {
        file << "// " << t.comment << "\n";
    }

    switch (t.category) {

    case Type::Category::Basetype:
        if (!t.deprecated.empty()) throw std::runtime_error("deprecated basetype not supported yet");
        _gen_arbitrary_C_code_in_type(type, file);
        break;
    case Type::Category::Bitmask:
        if (!t.deprecated.empty()) throw std::runtime_error("deprecated bitmask not supported yet");
        _gen_arbitrary_C_code_in_type(type, file); // bitvalues??
        break;
    case Type::Category::Define:
        if (!t.deprecated.empty()) throw std::runtime_error("deprecated define not supported yet");
        _gen_arbitrary_C_code_in_type(type, file);
        break;
    case Type::Category::Enum:
        file << "enum class ";
        if (!t.deprecated.empty()) file << " [[deprecated(\"" << t.deprecated << "\")]] ";
        file << t.name << ";\n";
        break;

    case Type::Category::Handle:
        if (!t.deprecated.empty()) throw std::runtime_error("deprecated handle not supported yet");
        if (type.children.size() == 0)
            _gen_arbitrary_C_code_in_type(type, file);
        break; // TODO: handle
    case Type::Category::Funcpointer:
        if (!t.deprecated.empty()) throw std::runtime_error("deprecated handle not supported yet");
        _gen_arbitrary_C_code_in_type(type, file);
        break;
    case Type::Category::Include:
        break;

    case Type::Category::Struct:
        break;
    case Type::Category::Union:
        break;
    case Type::Category::None:
        break;
    };

    if (!t.alias.empty()) {
        file << "using " << t.name;
        if (!t.deprecated.empty())
            file << " [[deprecated(\"" << t.deprecated << "\")]]";
        file << " = " << t.alias << ";\n";
    }


}

void generate_types(vkg_gen::xml::Dom& dom, std::ofstream& file) {
    auto& children = dom.root->asElement().children;
    auto it = std::ranges::find_if(children, [](Node* n) {
        return n->isElement() && n->asElement().tag == "types";
        });

    if (it == children.end()) {
        throw std::runtime_error("expected <types> tag");
    };

    for (; it != children.end(); ++it)
        for (Node* node : (*it)->asElement().children) {
            if (node->isText()) {
                std::cout << "- Skipping: " << node->asText() << std::endl;
                continue;
            }
            auto& node_elem = node->asElement();
            if (node_elem.tag != "type") {
                std::cout << "- Skipping: ";
                helper_test(std::cout, node) << std::endl;
                continue;
            }
            generate_type(node_elem, dom.arena, file);
        }
};


void generate_base_types(const vkg_gen::xml::Dom& dom, std::ofstream& file) {
    auto base_types = has_tag("type") && has_attr("category", "basetype");
    auto& children = dom.root->asElement().children;

    auto it = std::ranges::find_if(children, has_tag("types"));

    if (it == children.end()) {
        throw std::runtime_error("expected <types> tag");
    };

    for (; it != children.end(); ++it)
        for (Node* node : dom.children(*it) | std::views::filter(base_types)) {
            auto& type = node->asElement();

            for (Node* child : type.children) {
                if (child->isText()) {
                    file << child->asText();
                    continue;
                }
                auto& ch = child->asElement();
                if (ch.tag == "name") {
                    // TODO: register it
                    if (ch.children.size() != 1 || !ch.children[0]->isText()) {
                        throw std::runtime_error("expected <name> to have one text child");
                    };

                    file << ch.children[0]->asText();

                } else if (ch.tag == "type") {
                    // TODO: find it
                    if (ch.children.size() != 1 || !ch.children[0]->isText()) {
                        throw std::runtime_error("expected <name> to have one text child");
                    };

                    file << ch.children[0]->asText();

                } else {
                    std::cout << " - Skipping: ";
                    helper_test(std::cout, child) << '\n';
                }
            }
            file << '\n';
        }
}

void generate_enums(const vkg_gen::xml::Dom& dom, std::ofstream& file) {
    auto expr = has_tag("enums") && has_attr("type", "enum");

    for (Node* node : dom.children(dom.root) | std::views::filter(expr)) {
        auto& enum_ = node->asElement();
        file << "enum class " << enum_.get_attr_value("name") << " {\n";

        for (Node* val_ : enum_.children) {
            auto& val = val_->asElement();
            if (val.tag == "comment") {
                file << "    /* " << val.children[0]->asText() << " */\n";
                continue;
            }

            if (val.tag != "enum") {
                std::cout << " - Skipping: ";
                helper_test(std::cout, val_) << '\n';
                continue;
            }


            file << "    " << val.get_attr_value("name");
            if (!val.get_attr_value("deprecated").empty())
                file << "    [[deprecated(\"" << val.get_attr_value("deprecated") << "\")]] ";

            if (val.get_attr_value("value").empty()) {
                if (!val.get_attr_value("alias").empty())
                    file << " = " << val.get_attr_value("alias");
                else
                    throw std::runtime_error{ "No alias for enum value: " + std::string(val.get_attr_value("name")) };
            } else {
                file << " = " << val.get_attr_value("value");
            }


            if (val_ != enum_.children.back()) {
                file << ",";
            }

            if (!val.get_attr_value("comment").empty())
                file << " // " << val.get_attr_value("comment");
            file << '\n';
        }

        file << "};\n";
    }
}

template <std::ranges::input_range R>
std::ranges::range_value_t<R>* get_single_if_exists(R&& range) {
    auto it = std::ranges::begin(range);
    if (it == std::ranges::end(range)) return nullptr; // empty
    auto first = it;
    ++it;
    if (it != std::ranges::end(range)) return nullptr; // more than one
    return &(*first);
}

void generate_API_constants(const vkg_gen::xml::Dom& dom, std::ofstream& file) {
    auto expr = has_tag("enums") && has_attr("type", "constants");

    auto tmp = get_single_if_exists(dom.children(dom.root) | std::views::filter(expr));
    if (tmp == nullptr) {
        throw std::runtime_error{ "API constants must have only one element" };
    }

    auto& constants = (*tmp)->asElement();

    file << "// " << constants.get_attr_value("comment") << '\n';
    for (Node* constant : constants.children) {
        auto& c = constant->asElement();
        file << "constexpr " << c.get_attr_value("type") << " " << c.get_attr_value("name") << " = " << c.get_attr_value("value") << ";";
        if (!c.get_attr_value("comment").empty())
            file << " // " << c.get_attr_value("comment");
        file << '\n';
    }
    file << '\n';
}

TypeEnum::Type TypeEnum::type_from_string(std::string_view s) {
    if (s == "bitmask") return Type::Bitmask;
    if (s == "enum") return Type::Normal;
    if (s == "constants") return Type::Constants;
    throw std::runtime_error{ "Unknown type: " + std::string(s) };
}

TypeEnum::Bitwidth TypeEnum::bitwidth_from_string(std::string_view s) {
    if (s == "8") return Bitwidth::_8;
    if (s == "16") return Bitwidth::_16;
    if (s == "32") return Bitwidth::_32;
    if (s == "64") return Bitwidth::_64;
    throw std::runtime_error{ "Unknown bitwidth: " + std::string(s) };
}

TypeEnum::TypeEnum(const vkg_gen::xml::Element& e, vkg_gen::Arena& arena) {
    for (auto& attr : e.attrs) {
        if (attr.name == "name") {
            name = attr.value;
        } else if (attr.name == "comment") {
            comment = attr.value;
        } else if (attr.name == "type") {
            type = type_from_string(e.get_attr_value("type"));
        } else if (attr.name == "bitwidth") {
            bitwidth = bitwidth_from_string(attr.value);
        } else {
            throw std::runtime_error{ "Unknown attribute: " + std::string(attr.name) };
        }
    };

    if (name.empty())
        throw std::runtime_error{ "Enum must have a name" };

    if (type == Type::None)
        throw std::runtime_error{ "Enum must have a type" };

    if (bitwidth != Bitwidth::None && type == Type::Constants)
        throw std::runtime_error{ "Constants cannot have a bitwidth, they specify the type instead" };


    for (Node* child : e.children) {
        if (!child->isElement()) {
            std::cout << "skipping non-element child" << std::endl;
            continue;
        }

        auto& ch = child->asElement();

        if (ch.tag == "comment") {
            items.emplace_back(EnumItem::from_xml(ch, arena, this, true));
            continue;
        }

        if (ch.tag == "unused") {
            std::cout << "skipping unused" << std::endl;
            continue;
        }

        if (ch.tag != "enum")
            throw std::runtime_error("unknown child of <enum>: " + std::string(ch.tag));

        items.emplace_back(EnumItem::from_xml(ch, arena, this));
    };
};

//FIXME:
std::string to_string(TypeEnum::Bitwidth b) { return "TODO"; }
std::string to_string(TypeEnum::Type t) {
    switch (t) {
    case TypeEnum::Type::None: return "None";
    case TypeEnum::Type::Normal: return "enum";
    case TypeEnum::Type::Bitmask: return "bitmask";
    case TypeEnum::Type::Constants: return "constants";
    }
    //TODO:
    throw std::runtime_error{ "Unknown type" };
}

uint8_t bitpos_from_string(std::string_view s) {
    int value = 0;
    for (char c : s) {
        if (c < '0' || c > '9')
            throw std::runtime_error{ "Invalid bitpos: " + std::string(s) };
        value = value * 10 + (c - '0');
        if (value > BITPOS_MAX)
            throw std::runtime_error{ "Invalid bitpos: " + std::string(s) };
    }
    return value;
}

Member::LimitType Member::limit_type_from_string(std::string_view s) {
    uint16_t value = 0;

#define LIMIT(x, y) if (tok == x) { if (value & (uint16_t)LimitType::y) throw std::runtime_error{ "Duplicate limit of " x " in: " + std::string(s) } ; value |= (uint16_t)LimitType::y;}

    size_t start = 0;

    while (start < s.size()) {
        sv tok = s.substr(start, s.find(','));
        start += tok.size() + 1;

        LIMIT("min", Min)
else LIMIT("max", Max)
        else LIMIT("not", Not)
        else LIMIT("pot", Pot)
        else LIMIT("mul", Mul)
        else LIMIT("bits", Bits)
        else LIMIT("bitmask", Bitmask)
        else LIMIT("range", Range)
        else LIMIT("struct", Struct)
        else LIMIT("exact", Exact)
        else LIMIT("noauto", NoAuto)
        else
            throw std::runtime_error{ "Unknown limit type: " + std::string(s) };
    }
    return (Member::LimitType)value;
}

Member::Member(const vkg_gen::xml::Element& e, vkg_gen::Arena& arena, ParentType parent_type, bool is_standalone_comment) :
    element(e), is_standalone_comment(is_standalone_comment), stringify("") {
    if (is_standalone_comment) {
        assert(e.tag == "comment");
        comment = e.children[0]->asText();
        return;
    }

    assert(e.tag == "member");

    for (auto& attr : e.attrs) {
        if (attr.name == "name") {
            name = attr.value;
        } else if (attr.name == "api") {
            api = attr.value;
        } else if (attr.name == "comment") {
            comment = attr.value;
        } else if (attr.name == "len") {
            len = attr.value;
        } else if (attr.name == "altlen") {
            altlen = attr.value;
        } else if (attr.name == "stride") {
            stride = attr.value;
        } else if (attr.name == "deprecated") {
            deprecated = attr.value;
        } else if (attr.name == "externsync") {
            externsync = externsync_from_string(attr.value);
        } else if (attr.name == "optional") {
            optional = bool_from_string(attr.value);
        } else if (attr.name == "selector") {
            selector = attr.value;
        } else if (attr.name == "selection") {
            selection = attr.value;
        } else if (attr.name == "noautovalidity") {
            noautovalidity = bool_from_string(attr.value);
        } else if (attr.name == "values") {
            values = attr.value;
        } else if (attr.name == "limittype") {
            limittype = limit_type_from_string(attr.value);
        } else if (attr.name == "objecttype") {
            objecttype = attr.value;
        } else if (attr.name == "featurelink") {
            featurelink = attr.value;
        } else {
            throw std::runtime_error{ "Unknown attribute: " + std::string(attr.name) };
        }
    }

    for (Node* child : e.children) {
        if (child->isText()) {
            stringify += child->asText();
            continue;
        }
        auto& ch = child->asElement();
        if (ch.tag == "name") {
            if (!name.empty())
                throw std::runtime_error{ "Duplicate name in member: " + std::string(name) };
            name = ch.children[0]->asText(); // TODO: check_name
            stringify += " ";
            stringify += name;
        } else if (ch.tag == "type" || ch.tag == "enum") {
            if (type.empty()) {
                type = ch.children[0]->asText();
                stringify += type;
            } else {

                // THIS CAN HAPPEN: for example with "VkPipelineCacheHeaderVersionOne", <member><type>uint8_t</type><name>pipelineCacheUUID</name>[<enum>VK_UUID_SIZE</enum>]</member>
                std::cout << "FIXME: Duplicate type in member: " << std::string(name) << std::endl;
                stringify += ch.children[0]->asText();
            }

        } else if (ch.tag == "comment") {
            if (!comment.empty())
                throw std::runtime_error{ "Duplicate comment in member: " + std::string(comment) };
            comment = ch.children[0]->asText();
        } else {
            throw std::runtime_error{ "Unknown child: " + std::string(ch.tag) };
        }
    }
    stringify += ";";
}

TypeEnum::EnumItem TypeEnum::EnumItem::from_xml(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena, TypeEnum* parent, bool is_standalone_comment, bool extend_parent) {
    EnumItem item{ .is_standalone_comment = is_standalone_comment };

    assert(!(is_standalone_comment && extend_parent));

    if (is_standalone_comment) {
        assert(elem.tag == "comment");
        item.comment = elem.children[0]->asText();
        return item;
    }

    assert(elem.tag == "enum");

    bool value_specified = false;
    bool offset_specified = false;

    sv extnumber;
    sv dir;
    sv offset;

    for (auto& attr : elem.attrs) {
        if (attr.name == "name") {
            item.name = attr.value;
        } else if (attr.name == "comment") {
            item.comment = attr.value;
        } else if (attr.name == "api") {
            item.api = attr.value;
        } else if (attr.name == "protect") {
            item.protect = attr.value;
        } else if (attr.name == "deprecated") {
            item.deprecated = attr.value;
        } else if (attr.name == "alias") {
            if (value_specified)
                throw std::runtime_error{ "Enum item cannot have both 'value/bitpos' and 'alias'" };
            if (offset_specified)
                throw std::runtime_error{ "Enum item cannot have both 'alias' and 'extnumber/dir/offset'" };

            item.alias = attr.value;
            item.is_alias = true;
        } else if (attr.name == "value") {
            if (item.is_alias)
                throw std::runtime_error{ "Enum item cannot have both 'value' and 'alias'" };
            if (value_specified)
                throw std::runtime_error{ "Enum item cannot have both 'value' and 'bitpos'" };
            if (offset_specified)
                throw std::runtime_error{ "Enum item cannot have both 'value' and 'extnumber/dir/offset'" };

            if (parent->type == Type::Constants)
                item.constant.value = attr.value;
            else if (parent->type == Type::Normal)
                item.normal.value = attr.value;
            else if (parent->type == Type::Bitmask)
                item.bitmask.value = attr.value;
            else
                assert(false);

            value_specified = true;

        } else if (attr.name == "bitpos") {
            if (item.is_alias)
                throw std::runtime_error{ "Enum item cannot have both 'bitpos' and 'alias'" };
            if (value_specified)
                throw std::runtime_error{ "Enum item cannot have both 'bitpos' and 'value'" };
            if (offset_specified)
                throw std::runtime_error{ "Enum item cannot have both 'bitpos' and 'extnumber/dir/offset'" };

            if (parent->type != Type::Bitmask)
                throw std::runtime_error{ "Invalid 'bitpos' attribute for enum type: " + to_string(parent->type) };
            item.bitmask.bitpos = bitpos_from_string(attr.value);
            item.bitmask.is_bitfield = true;
            // TODO: check range with bitwidth
            // TODO: remove from_chars and write own parser
            value_specified = true;

        } else if (attr.name == "type") {
            if (parent->type != Type::Constants)
                throw std::runtime_error{ "Invalid 'type' attribute for enum type: " + to_string(parent->type) };
            assert(parent->bitwidth == Bitwidth::None);
            item.constant.type = attr.value; // TODO: parse
        } else if (extend_parent && (attr.name == "extends" || attr.name == "extnumber" || attr.name == "dir" || attr.name == "offset")) {
            if (attr.name != "extends") {
                offset_specified = true;
                if (value_specified)
                    throw std::runtime_error{ "Enum item cannot have both 'value/bitpos' and 'extnumber/dir/offset'" };
                if (item.is_alias)
                    throw std::runtime_error{ "Enum item cannot have both 'alias' and 'extnumber/dir/offset'" };
            }

            if (attr.name == "extnumber")
                extnumber = attr.value;
            else if (attr.name == "dir")
                dir = attr.value;
            else if (attr.name == "offset")
                offset = attr.value;
        } else {
            throw std::runtime_error{ "Unknown attribute: " + std::string(attr.name) };

        }
    };

    if (item.name.empty())
        throw std::runtime_error{ "Enum item must have a name" };



    if (extend_parent) {
        if (!value_specified && !item.is_alias && !offset_specified)
            // TODO: add information on where wre we and some enum info
            throw std::runtime_error{ "Enum item must have'value'/'bitpos', 'alias' or 'offset' attribute when extending parent." };
    } else {
        if (!value_specified && !item.is_alias)
            throw std::runtime_error{ "Enum item must have 'value'/'bitpos' or 'alias' attribute" };
    }

    if (offset_specified) {
        std::cout << "FIXME: Extending with 'extnumber/dir/offset' is not supported yet\n";
        // Handle extend with 'extnumber/dir/offset'
        item.comment = arena.storeString("Value incorrect. Extending with 'extnumber/dir/offset' is not supported yet!");
        sv value = arena.storeString("0");

        if (parent->type == Type::Constants)
            item.constant.value = value;
        else if (parent->type == Type::Normal)
            item.normal.value = value;
        else if (parent->type == Type::Bitmask)
            item.bitmask.value = value;
    }

    return item;
}



void Generator::parse_types(vkg_gen::xml::Dom& dom, std::ofstream& file) {
    auto& registry = dom.root->asElement();
    assert(registry.tag == "registry");

    // TODO: platforms
    // TODO: tags

    // TODO: there should be only one
    auto all_types = registry.children | std::views::filter(has_tag("types"));
    for (Node* types : all_types) {
        for (Node* child : types->asElement().children) {
            // TODO: grouping?? and comments tag
            if (child->isText()) {
                std::cout << "- Skipping TEXT: " << child->asText() << std::endl;
                continue;
            }

            auto& type = child->asElement();
            if (type.tag != "type") {
                std::cout << "- FIXME: ";
                helper_test(std::cout, child) << std::endl;
                continue;
            };

            Type t(type, dom.arena);
            std::cout << "- " << t.name << std::endl;
            auto [it, inserted] = this->types.emplace(t.name, std::move(t));
            if (inserted) {
                this->types_flat_ordered.push_back(&it->second);
            }

        }
    }

}

void Generator::parse_enums(vkg_gen::xml::Dom& dom) {
    auto& registry = dom.root->asElement();
    assert(registry.tag == "registry");

    for (Node* type : registry.children | std::views::filter(has_tag("enums"))) {
        TypeEnum e(type->asElement(), dom.arena);
        std::cout << "- " << e.name << std::endl;
        this->enums.emplace(e.name, std::move(e));
    }
}

constexpr std::string_view bitwidth_to_str_type(TypeEnum::Bitwidth w) {
    switch (w) {
    case TypeEnum::Bitwidth::None:
        return "";
    case TypeEnum::Bitwidth::_8:
        return ": uint8_t ";
    case TypeEnum::Bitwidth::_16:
        return ": uint16_t ";
    case TypeEnum::Bitwidth::_32:
        return ": uint32_t ";
    case TypeEnum::Bitwidth::_64:
        return ": uint64_t ";
    }
}



void Generator::generate_enum_alias(Type& e, std::ofstream& file) {
    if (e.alias.empty())
        return;

    file << LineComment{ e.comment, true };
    file << "using " << e.name << Deprecate{ e.deprecated } << "= " << e.alias << ";\n\n";
}

void Generator::generate_enum(TypeEnum& e, std::ofstream& file) {
    file << LineComment{ e.comment, true };

    if (e.type == TypeEnum::Type::Constants) {
        for (auto& item : e.items) {
            file << "constexpr " << item.constant.type << " " << item.name << " = " << item.constant.value << ";"
                << LineComment{ item.comment } << '\n';
        }
        file << '\n';
        return;
    }

    file << "enum class" << Deprecate{ e.deprecated } << e.name << " " << bitwidth_to_str_type(e.bitwidth) << " {\n";

    for (auto& item : e.items) {
        if (item.is_standalone_comment) {
            file << "/* " << item.comment << " */\n";
            continue;
        }

        file << "    " << item.name << Deprecate{ item.deprecated } << "= ";

        if (item.is_alias) {
            file << item.alias;
        } else if (e.type == TypeEnum::Type::Normal) {
            file << item.normal.value;
        } else if (e.type == TypeEnum::Type::Bitmask) {
            if (item.bitmask.is_bitfield) {
                if (item.bitmask.bitpos == 31)
                    file << "1U";
                else if (item.bitmask.bitpos > 31)
                    file << "1ULL";
                else
                    file << "1";

                file << " << " << (int)item.bitmask.bitpos;
            } else {
                file << item.bitmask.value;
            }

        } else {
            throw std::runtime_error("TODO: internal error");
            assert(false);
        }

        file << "," << LineComment{ item.comment } << '\n';
    }
    file << "};\n\n";
}

void Generator::generate_struct(Type& s, std::ofstream& file) {
    assert(s.category == Type::Category::Struct);

    std::cout << "Generating struct: " << s.name << std::endl;

    file << LineComment{ s.comment };

    if (!s.alias.empty()) {
        file << "using " << s.name << Deprecate{ s.deprecated } << "= " << s.alias << ";\n";
        return;
    }

    file << "struct" << Deprecate{ s.deprecated } << s.name << " {\n";

    for (auto& member : s.struct_->members) {
        if (!member.api.empty() && member.api != "vulkan") {
            std::cout << " TODO: skipping member due to api(could be duplicated): " << member.stringify << "\n";
            continue;
        }

        file << "    " << member.stringify << LineComment{ member.comment, false } << '\n';
    }
    file << "};\n\n";
}

void Generator::generate_union(Type& s, std::ofstream& file) {
    assert(s.category == Type::Category::Union);

    std::cout << "Generating union: " << s.name << std::endl;

    file << LineComment{ s.comment };

    if (!s.alias.empty()) {
        file << "using " << s.name << Deprecate{ s.deprecated } << "= " << s.alias << ";\n";
        return;
    }

    file << "union" << Deprecate{ s.deprecated } << s.name << " {\n";

    for (auto& member : s.union_->members) {
        file << "    " << member.stringify << LineComment{ member.comment, false } << '\n';
    }
    file << "};\n\n";
}

void Generator::generate_bitmask(Type& bitmask, std::ofstream& file) {
    file << LineComment{ bitmask.comment, true };

    file << "using " << bitmask.name << Deprecate{ bitmask.deprecated } << "= ";
    if (!bitmask.alias.empty()) {
        file << bitmask.alias << ";\n";

    } else {
        // CHECK: revisit this
        auto it = std::ranges::find_if(bitmask.elem.children, has_tag("type"));
        if (it == bitmask.elem.children.end())
            throw std::runtime_error{ "bitmask type not found" };

        file << (*it)->asElement().children[0]->asText() << ";\n"; // TODO: check children
        std::cout << "bitmask: " << bitmask.name << " = " << (*it)->asElement().children[0]->asText() << std::endl;
    }

};



void Generator::generate_handle(Type& h, std::ofstream& file, TypeEnum& obj_enum) {
    assert(h.category == Type::Category::Handle);

    auto it = std::ranges::find(obj_enum.items, h.handle->objtypeenum, &TypeEnum::EnumItem::name);
    if (it == obj_enum.items.end())
        std::cout << "FIXME: Handle '" << h.name << "' did not found matching objtypeenum '" << h.handle->objtypeenum << "' in enum '" << obj_enum.name << "'" << std::endl;
    //throw std::runtime_error{ std::format("Handle '{}' did not found matching objtypeenum '{}' in enum '{}'", h.name, h.handle->objtypeenum, obj_enum.name) };

    file << "using " << h.name << Deprecate{ h.deprecated } << "= "
        << "Handle<struct " << obj_enum.name << "_T*>" << ";" << LineComment{ h.comment } << '\n';
}

void Generator::add_required_type(sv name) {
    std::vector<Type*> required_types;
    add_required_type(name, required_types);
    assert(required_types.size() == 0);
}

void Generator::add_required_type(sv name, std::vector<Type*>& required_types) {
    Type* type = &types.find(name)->second;
    if (std::ranges::find(required_types, type) == required_types.end())
        required_types.push_back(type);
    else {
        std::cout << "FIXME: Returning from circular dependency: " << name << "\n";
        return;
    }


    using sv = std::string_view;


    // TODO: Write own split
    for (auto&& part : type->requires_ | std::views::split(',')) {
        sv req(&*part.begin(), std::ranges::distance(part));
        if (req.empty()) {
            std::cout << "FIXME: empty requires NOT HANDLED" << std::endl;
            break;
        }

        // FIXME: Circular dependency
        if (index.find(req) == index.end()) {
            add_required_type(req, required_types);
        }

    }

    switch (type->category) {

        /// These types do not require any special handling
    case Type::Category::Basetype: // fallthrough
    case Type::Category::Bitmask:  // fallthrough
    case Type::Category::Define:   // fallthrough
    case Type::Category::Enum:     // fallthrough
        // CHECK: handle too?
    case Type::Category::Handle:   // fallthrough
    case Type::Category::Include:  // fallthrough
    case Type::Category::None:
        break;

    case Type::Category::Struct:   // fallthrough
    case Type::Category::Union:
        for (auto& member : type->struct_->members) {
            if (member.is_standalone_comment)
                continue;
            if (index.find(member.type) == index.end()) {
                add_required_type(member.type, required_types);
            }
        }

        // CHECK: Im not sure about this logic. What if we have struct, that is defined and want to also be accessed by this alias?
        if (!type->alias.empty() && index.find(type->alias) == index.end()) {
            add_required_type(type->alias, required_types);
        }

        break;

    case Type::Category::Funcpointer:
        // FIXME: function pointer not handled
        break;

    }

    required_types.pop_back();
    index[type->name] = required_types_ordered.size();
    required_types_ordered.push_back(type);
}


void Generator::add_required_feature(Element& feature, vkg_gen::Arena& arena) {
    // TODO: check arguments of the feature.

    for (Node* node : feature.children) {
        if (node->isText()) {
            std::cout << "- Skipping: " << node->asText() << std::endl;
            continue;
        }

        auto& elem = node->asElement();
        if (elem.tag == "require") {
            for (Node* type : elem.children) {
                if (type->isText()) {
                    std::cout << "- Skipping TEXT: " << type->asText() << std::endl;
                    continue;
                }

                Element& elem = type->asElement();

                if (elem.tag == "type") {
                    sv name = elem.get_attr_value("name");
                    if (index.find(name) == index.end()) {
                        // CHECK: are type aliases handled correctly?
                        add_required_type(name);
                    }
                } else if (elem.tag == "enum") {
                    auto it = std::ranges::find(elem.attrs, "extends", &Attribute::name);
                    if (it != elem.attrs.end()) {
                        extend_enum(it->value, elem, arena);
                    }
                    // Currently generating all enums

                } else if (elem.tag == "command") {
                    // Currently ignoring commands

                } else {
                    std::cout << "- FIXME: currently ignoring: ";
                    helper_test(std::cout, type) << std::endl;
                    continue;
                }



            }

        }
    }
}

void Generator::extend_enum(sv extends, Element& elem, vkg_gen::Arena& arena) {

    TypeEnum& enum_ = enums.find(extends)->second;
    enum_.items.emplace_back(TypeEnum::EnumItem::from_xml(elem, arena, &enum_, false, true));
}

void Generator::generate(vkg_gen::xml::Dom& dom, std::ofstream& file, void* config) {



#if 1
    file << "#pragma once\n";
    file << boilerplate::VIDEO_INCLUDES << "\n";

    file << "#define VKAPI_PTR\n";

    file << "typedef uint32_t VkFlags;\n";
    file << "typedef uint64_t VkFlags64;\n";

    file << boilerplate::HANDLE_DEFINITION << "\n";

    //generate_API_constants(dom, file);
    //generate_types(dom, file);

    parse_types(dom, file);
    parse_enums(dom);

    auto it = std::ranges::find_if(dom.root->asElement().children, has_tag("feature") && has_attr("name", "VK_VERSION_1_0"));
    if (it == dom.root->asElement().children.end()) {
        throw std::runtime_error{ "VK_VERSION_1_0 not found" };
    }
    add_required_feature((*it)->asElement(), dom.arena);



    it = std::ranges::find_if(dom.root->asElement().children, has_tag("feature") && has_attr("name", "VK_VERSION_1_1"));
    if (it == dom.root->asElement().children.end()) {
        throw std::runtime_error{ "VK_VERSION_1_0 not found" };
    }
    add_required_feature((*it)->asElement(), dom.arena);


    for (auto& [_, enum_] : enums) {
        generate_enum(enum_, file);
    }

    for (Type* p : required_types_ordered) {
        auto& type = *p;
        if (exclude_platforms(type.name)) {
            std::cout << "Excluding: " << type.name << std::endl;
            continue;
        }

        switch (type.category) {
        case Type::Category::Struct:
            generate_struct(type, file);
            break;
        case Type::Category::Union:
            generate_union(type, file);
            break;
        case Type::Category::Enum:
            generate_enum_alias(type, file);
            break;

        case Type::Category::Bitmask:
            generate_bitmask(type, file);
            break;
            // TODO: provizorní řešení
        case Type::Category::Basetype:
            //case Type::Category::Define:
        case Type::Category::Funcpointer:
            _gen_arbitrary_C_code_in_type(type.elem, file);
            break;
        case Type::Category::Handle:
            generate_handle(type, file, enums.at(TypeHandle::obj_enum_name));
            break;
        default:
            break;
        }
    }

#else
    //auto unions_filter = has_tag("type") && has_attr("category", "union");
    //auto types = dom.getChildrenByTag("types")[0]->asElement();
//
    //for (const auto& child : types.children | std::views::filter(unions_filter)) {
    //    std::cout << "<type ";
    //    for (auto& attr : child->asElement().attrs) {
    //        std::cout << attr.name << "='" << attr.value << "', ";
    //    }
    //    std::cout << ">\n";
//
    //};

    auto extension_filter = has_tag("extension");
    auto extensions = dom.getChildrenByTag("extensions")[0]->asElement();
    for (Node* child : extensions.children | std::views::filter(extension_filter)) {
        auto& ch = child->asElement();
        std::cout << ch.get_attr_value("name") << " = " << ch.get_attr_value("number") << ",\n";
    }
#endif
}

