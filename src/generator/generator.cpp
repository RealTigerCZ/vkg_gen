/**
 * @file generator.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief
 * @date Created: 12. 11. 2025
 * @date Modified: 25. 02. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */


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
    // author: PCJohn (peciva at fit.vut.cz)
    static const char* HANDLE_DEFINITION = ""
        "// FIXME: \n"
        "template<typename Type> class UniqueHandle;\n"
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

    // FIXME:
    static const char* HEADER_EXTERNS = ""
        "extern void load_lib(const char* path);\n"
        "extern void unload_lib();\n"
        "extern void init_vk_functions();\n";

    static const char* CPP_IMPL = ""
        "#include <dlfcn.h>\n"
        "#include <assert.h>\n"
        "\n"
        "void* lib = nullptr;\n"
        "VkInstance instance = nullptr;\n"
        "FuncTable funcs;\n"
        "static VkExtensionProperties * extensions = nullptr;\n"
        "static const char** extensions_names = nullptr;\n"
        "static uint32_t extensions_count = 0;\n";

    static const char* LOAD_UNLOAD_LIB_IMPL = ""
        "void load_lib(const char* path) {\n"
        "    lib = dlopen(path, RTLD_NOW);\n"
        "    assert(lib);\n"
        "    funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(dlsym(lib, \"vkGetInstanceProcAddr\"));\n"
        "    assert(funcs.vkGetInstanceProcAddr);\n"
        "    funcs.vkCreateInstance = PFN_vkCreateInstance(funcs.vkGetInstanceProcAddr(nullptr, \"vkCreateInstance\"));\n"
        "    assert(funcs.vkCreateInstance);\n"
        "    funcs.vkEnumerateInstanceExtensionProperties = PFN_vkEnumerateInstanceExtensionProperties(funcs.vkGetInstanceProcAddr(nullptr, \"vkEnumerateInstanceExtensionProperties\"));\n"
        "    assert(funcs.vkEnumerateInstanceExtensionProperties);\n"
        "\n"
        "    funcs.vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);\n"
        "    extensions = new VkExtensionProperties[extensions_count];\n"
        "    extensions_names = new const char* [extensions_count];\n"
        "    funcs.vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, extensions);\n"
        "\n"
        "    for (uint32_t i = 0; i < extensions_count; i++) {\n"
        "        extensions_names[i] = extensions[i].extensionName;\n"
        "    }\n"
        "\n"
        "    VkApplicationInfo app_info = {\n"
        "                .pApplicationName = \"test\",\n"
        "                .applicationVersion = 0,\n"
        "                .pEngineName = nullptr,\n"
        "                .engineVersion = 0,\n"
        "                .apiVersion = 1 << 22 };\n"
        "\n"
        "    VkInstanceCreateInfo info = {\n"
        "                .flags = {},\n"
        "                .pApplicationInfo = &app_info,\n"
        "                .enabledLayerCount = 0,\n"
        "                .ppEnabledLayerNames = nullptr,\n"
        "                .enabledExtensionCount = extensions_count,\n"
        "                .ppEnabledExtensionNames = extensions_names };\n"
        "\n"
        "    funcs.vkCreateInstance(&info, nullptr, &instance);\n"
        "    assert(instance);\n"
        "}\n"
        "\n"
        "void unload_lib() {\n"
        "    if (!lib) return;\n"
        "    if (instance)\n"
        "       funcs.vkDestroyInstance(instance, nullptr);\n"
        "    dlclose(lib);\n"
        "    lib = nullptr;\n"
        "    instance = nullptr;\n"
        "}\n";
}

//FIXME:
using namespace vkg_gen::xml;

using namespace vkg_gen::Generator;

// TASK: 090126_01
#ifdef CONCEPT_FILTERING
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

template <NodePredicate P>
struct Not {
    P pred;
    inline constexpr bool operator()(const Node* n) const noexcept {
        return !pred(n);
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

template <NodePredicate P>
inline constexpr auto operator!(P pred) noexcept {
    return Not<P>{pred};
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

#endif // CONCEPT_FILTERING
namespace vkg_gen::xml {

    // TASK: 090126_03
    std::ostream& operator<<(std::ostream& os, const Element& elem) {
        os << "<" << elem.tag;
        for (auto& attr : elem.attrs) {
            os << " " << attr.name << "=\"" << attr.value << '"';
        }
        return os << ">";
    }
}

// TASK: 090126_03
std::ostream& helper_test(std::ostream& os, const vkg_gen::xml::Node* node) {
    if (!node) return os << "(NULL)";
    if (!node->isElement())
        return os << "(TEXT: " << node->asText() << ")";

    return os << node->asElement();
}

bool vkg_gen::Generator::bool_from_string(std::string_view s) {
    if (s == "true")
        return true;
    if (s == "false")
        return false;

    if (s.find(",") != std::string_view::npos) {
        std::cout << "FIXME: " << s << "not supported" << std::endl;
        return false;
    }

    throw my_error("Invalid boolean value: '" + std::string(s) + "'");
}

Command::Scope vkg_gen::Generator::scope_from_string(std::string_view s) {
    if (s == "inside") return Command::Scope::Inside;
    if (s == "outside") return Command::Scope::Outside;
    if (s == "both") return Command::Scope::Both;
    throw my_error("Invalid scope: '" + std::string(s) + "'");
}

Member::ExternSync externsync_from_string(std::string_view s) {
    if (s == "true")
        return Member::ExternSync::True;
    if (s == "false")
        return Member::ExternSync::False;
    if (s == "maybe")
        return Member::ExternSync::Maybe;

    throw my_error("Invalid externsync value: '" + std::string(s) + "'");
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

    std::stringstream err;
    err << "Invalid category: '" << s << "' {";
    for (auto c : s)
        err << '\'' << c << "\', ";
    err << "}";

    throw my_error(err);
}

void Type::parse_struct(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena) {
    for (Node* child : elem.children) {
        if (child->isText()) {
            std::cout << "- TASK: 090126_04 - Skipping TEXT: " << child->asText() << std::endl;
            continue;
        }

        auto& ch = child->asElement();

        if (ch.tag == "comment")
            struct_->members.emplace_back(Member(ch, arena, Member::ParentType::Struct, true));
        else if (ch.tag == "member")
            struct_->members.emplace_back(Member(ch, arena, Member::ParentType::Struct));
        else {
            throw my_error{ "Unknown child element: " + std::string(ch.tag) };
        }
    }

};


void Type::parse_union(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena) {
    for (Node* child : elem.children) {
        if (child->isText()) {
            std::cout << "- TASK: 090126_04 - Skipping TEXT: " << child->asText() << std::endl;
            continue;
        }

        auto& ch = child->asElement();

        if (ch.tag == "comment")
            union_->members.emplace_back(Member(ch, arena, Member::ParentType::Union, true));
        else if (ch.tag == "member")
            union_->members.emplace_back(Member(ch, arena, Member::ParentType::Union));
        else {
            throw my_error{ "Unknown child element: " + std::string(ch.tag) };
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
                throw my_error("Invalid combination of attributes and category on <type>");

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
                throw my_error("unexpected attribute on <type>");
            }

            if (attr.name == "parent")
                handle->parent = attr.value;
            else
                handle->objtypeenum = attr.value;
        } else if (attr.name == "returnedonly") {
            if (specific_attr == Category::None) {
                // TODO: this is a hack, and duplicates code
                auto it = std::ranges::find(elem.attrs, "category", &Attribute::name);
                if (it == elem.attrs.end())
                    throw my_error("unexpected attribute on <type>");
                specific_attr = category_from_string(it->value);

                if (specific_attr == Category::Struct) {
                    struct_ = arena.make<TypeStruct>();
                    struct_->returned_only = bool_from_string(attr.value);

                } else if (specific_attr == Category::Union) {
                    union_ = arena.make<TypeUnion>();
                    union_->returned_only = bool_from_string(attr.value);

                } else {
                    throw my_error("unexpected attribute on <type>");
                }


            } else if (specific_attr == Category::Struct) {
                struct_->returned_only = bool_from_string(attr.value);
            } else if (specific_attr == Category::Union) {
                union_->returned_only = bool_from_string(attr.value);
            } else {
                throw my_error("unexpected attribute on <type>");
            }



        } else if (attr.name == "structextends" || attr.name == "allowduplicate") {
            if (specific_attr == Category::None) {
                struct_ = arena.make<TypeStruct>();
            } else if (specific_attr != Category::Struct && specific_attr != Category::Union) {
                throw my_error("unexpected attribute on <type>");
            }

            if (attr.name == "structextends")
                struct_->struct_extends = attr.value;
            else if (attr.name == "allowduplicate")
                struct_->allow_duplicate = bool_from_string(attr.value);
            else
                struct_->returned_only = bool_from_string(attr.value);
        } else if (attr.name == "bitvalues") {
            if (specific_attr != Category::None && specific_attr != Category::Bitmask) {
                throw my_error("unexpected attribute on <type>");
            }

            specific_attr = Category::Bitmask;
            bitvalues = attr.value;
        } else {
            throw my_error("unknown attribute on <type>: " + std::string(attr.name));
        }

    }

    if (specific_attr == Category::Handle && handle->objtypeenum.empty() && alias.empty())
        throw my_error("objtypeenum attribute is required on <type category=\"handle\"> if alias attribute is not present");


    if (specific_attr == Category::Struct)
        parse_struct(elem, arena);

    if (specific_attr == Category::Union)
        parse_union(elem, arena);

    // TODO: check which types can have arbutrary C code
    if (name.empty()) {
        auto it = std::ranges::find_if(elem.children, has_tag("name"));
        if (it != elem.children.end()) {
            name = (*it)->asElement().children[0]->asText();
        } else {
            throw my_error("name attribute or <name> tag is required in <type>");
        }
    };
}

// TASK: 090126_03
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


// TASK: 090126_05
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

TypeEnum::Type TypeEnum::type_from_string(std::string_view s) {
    if (s == "bitmask") return Type::Bitmask;
    if (s == "enum") return Type::Normal;
    if (s == "constants") return Type::Constants;
    throw my_error{ "Unknown type of enum: " + std::string(s) };
}

TypeEnum::Bitwidth TypeEnum::bitwidth_from_string(std::string_view s) {
    if (s == "8") return Bitwidth::_8;
    if (s == "16") return Bitwidth::_16;
    if (s == "32") return Bitwidth::_32;
    if (s == "64") return Bitwidth::_64;
    throw my_error{ "Unknown bitwidth: " + std::string(s) };
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
            throw my_error{ "Unknown attribute: " + std::string(attr.name) };
        }
    };

    if (name.empty())
        throw my_error{ "Enum must have a name" };

    if (type == Type::None)
        throw my_error{ "Enum must have a type" };

    if (bitwidth != Bitwidth::None && type == Type::Constants)
        throw my_error{ "Constants cannot have a bitwidth, they specify the type instead" };


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
            throw my_error("unknown child of <enum>: " + std::string(ch.tag));

        items.emplace_back(EnumItem::from_xml(ch, arena, this));
    };
};

std::string to_string(TypeEnum::Bitwidth b) {
    switch (b) {
    case TypeEnum::Bitwidth::_8: return "8";
    case TypeEnum::Bitwidth::_16: return "16";
    case TypeEnum::Bitwidth::_32: return "32";
    case TypeEnum::Bitwidth::_64: return "64";
    case TypeEnum::Bitwidth::None: return "None";
    }
    // TODO: UNREACHABLE
    throw my_error{ "Unknown bitwidth" };
}
std::string to_string(TypeEnum::Type t) {
    switch (t) {
    case TypeEnum::Type::None: return "None";
    case TypeEnum::Type::Normal: return "enum";
    case TypeEnum::Type::Bitmask: return "bitmask";
    case TypeEnum::Type::Constants: return "constants";
    }
    //TODO: UNREACHABLE
    throw my_error{ "Unknown type" };
}

uint8_t bitpos_from_string(std::string_view s) {
    int value = 0;
    for (char c : s) {
        if (c < '0' || c > '9')
            throw my_error{ "Invalid bitpos: " + std::string(s) };
        value = value * 10 + (c - '0');
        if (value > BITPOS_MAX)
            throw my_error{ "Invalid bitpos: " + std::string(s) };
    }
    return value;
}

Member::LimitType Member::limit_type_from_string(std::string_view s) {
    uint16_t value = 0;

#define LIMIT(x, y) (tok == x) { if (value & (uint16_t)LimitType::y) throw my_error{ "Duplicate limit of " x " in: " + std::string(s) } ; value |= (uint16_t)LimitType::y;}

    size_t start = 0;

    while (start < s.size()) {
        sv tok = s.substr(start, s.find(','));
        start += tok.size() + 1;

        if LIMIT("min", Min)
        else if LIMIT("max", Max)
        else if LIMIT("not", Not)
        else if LIMIT("pot", Pot)
        else if LIMIT("mul", Mul)
        else if LIMIT("bits", Bits)
        else if LIMIT("bitmask", Bitmask)
        else if LIMIT("range", Range)
        else if LIMIT("struct", Struct)
        else if LIMIT("exact", Exact)
        else if LIMIT("noauto", NoAuto)
        else
            throw my_error{ "Unknown limit type: " + std::string(s) };
    }
    return (Member::LimitType)value;
#undef LIMIT
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
            throw my_error{ "Unknown attribute for member: " + std::string(attr.name) };
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
                throw my_error{ "Duplicate name in member: " + std::string(name) };
            name = ch.children[0]->asText(); // TODO: check_name
            stringify += " ";
            stringify += name;
        } else if (ch.tag == "type" || ch.tag == "enum") {
            if (type.empty()) {
                type = ch.children[0]->asText();
                stringify += type;
            } else {
                // TASK: 090126_08 - THIS CAN HAPPEN
                std::cout << "FIXME: Duplicate type in member: " << std::string(name) << std::endl;
                stringify += ch.children[0]->asText();
            }

        } else if (ch.tag == "comment") {
            if (!comment.empty())
                throw my_error{ "Duplicate comment in member: " + std::string(comment) };
            comment = ch.children[0]->asText();
        } else {
            throw my_error{ "Unknown child: " + std::string(ch.tag) };
        }
    }
    stringify += ";";
}

TypeEnum::EnumItem TypeEnum::EnumItem::from_xml(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena, TypeEnum* parent, bool is_standalone_comment, bool extend_parent, sv block_ext_number) {
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
                throw my_error{ "Enum item cannot have both 'value/bitpos' and 'alias'" };
            if (offset_specified)
                throw my_error{ "Enum item cannot have both 'alias' and 'extnumber/dir/offset'" };

            item.alias = attr.value;
            item.is_alias = true;
        } else if (attr.name == "value") {
            if (item.is_alias)
                throw my_error{ "Enum item cannot have both 'value' and 'alias'" };
            if (value_specified)
                throw my_error{ "Enum item cannot have both 'value' and 'bitpos'" };
            if (offset_specified)
                throw my_error{ "Enum item cannot have both 'value' and 'extnumber/dir/offset'" };

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
                throw my_error{ "Enum item cannot have both 'bitpos' and 'alias'" };
            if (value_specified)
                throw my_error{ "Enum item cannot have both 'bitpos' and 'value'" };
            if (offset_specified)
                throw my_error{ "Enum item cannot have both 'bitpos' and 'extnumber/dir/offset'" };

            if (parent->type != Type::Bitmask)
                throw my_error{ "Invalid 'bitpos' attribute for enum type: " + to_string(parent->type) };
            item.bitmask.bitpos = bitpos_from_string(attr.value);
            item.bitmask.is_bitfield = true;
            // TODO: check range with bitwidth
            // TODO: remove from_chars and write own parser
            value_specified = true;

        } else if (attr.name == "type") {
            if (parent->type != Type::Constants)
                throw my_error{ "Invalid 'type' attribute for enum type: " + to_string(parent->type) };
            assert(parent->bitwidth == Bitwidth::None);
            item.constant.type = attr.value; // TODO: parse
        } else if (extend_parent && (attr.name == "extends" || attr.name == "extnumber" || attr.name == "dir" || attr.name == "offset")) {
            if (attr.name != "extends") {
                offset_specified = true;
                if (value_specified)
                    throw my_error{ "Enum item cannot have both 'value/bitpos' and 'extnumber/dir/offset'" };
                if (item.is_alias)
                    throw my_error{ "Enum item cannot have both 'alias' and 'extnumber/dir/offset'" };
            }

            if (attr.name == "extnumber")
                extnumber = attr.value;
            else if (attr.name == "dir")
                dir = attr.value;
            else if (attr.name == "offset")
                offset = attr.value;
        } else {
            throw my_error{ "Unknown attribute: " + std::string(attr.name) };

        }
    };

    if (item.name.empty())
        throw my_error{ "Enum item must have a name" };



    if (extend_parent) {
        if (!value_specified && !item.is_alias && !offset_specified)
            // TODO: add information on where wre we and some enum info
            throw my_error{ "Enum item must have'value'/'bitpos', 'alias' or 'offset' attribute when extending parent." };
    } else {
        if (!value_specified && !item.is_alias)
            throw my_error{ "Enum item must have 'value'/'bitpos' or 'alias' attribute" };
    }

    if (offset_specified) {
        if (parent->type != Type::Normal)
            throw my_error{ "Offset/extnumber/dir can only be used with 'normal' enum type, not bitmask or constant." };

        if (extnumber.empty())
            if (block_ext_number.empty())
                throw my_error{ "Cannot infer extnumber when extending enum with offset! (Move this enum to <extension> tag or add 'extnumber' attribute)" };
            else
                extnumber = block_ext_number;

        int ext_number = std::stoi(extnumber.data()) - 1;
        int offset_i = std::stoi(offset.data());
        if (offset_i > 1000) // 3 digits
            throw my_error{ "Offset must be less than 1000" };

        // Vulkan spec:
        long normal_value = 1000'000'000 + ext_number * 1000 + offset_i;
        item.normal.value = arena.storeString(std::string(dir) + std::to_string(normal_value));
    }

    return item;
}



void Generator::parse_types(vkg_gen::xml::Dom& dom) {
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
            this->types.emplace(t.name, std::move(t));

        }
    }

}

void Generator::parse_enums(vkg_gen::xml::Dom& dom) {
    auto& registry = dom.root->asElement();
    assert(registry.tag == "registry");

    for (Node* enum_ : registry.children | std::views::filter(has_tag("enums"))) {
        TypeEnum e(enum_->asElement(), dom.arena);
        std::cout << "- " << e.name << std::endl;
        this->enums.emplace(e.name, std::move(e));
    }
}

void vkg_gen::Generator::Generator::parse_commands(vkg_gen::xml::Dom& dom) {
    auto& registry = dom.root->asElement();
    assert(registry.tag == "registry");

    auto all_commands = registry.children | std::views::filter(has_tag("commands"));
    for (Node* cmds : all_commands) {
        for (Node* child : cmds->asElement().children) {
            if (child->isText()) {
                std::cout << " TASK: 090126_04 - Skipping TEXT: " << child->asText() << std::endl;
                continue;
            }

            auto& cmd = child->asElement();
            if (cmd.tag != "command") {
                std::cout << "- FIXME: ";
                helper_test(std::cout, child) << std::endl;
                continue;
            };
            //FIXME: aliased commands
            if (std::ranges::find(cmd.attrs, "alias", &Attribute::name) != cmd.attrs.end()) {
                CommandAlias ca = CommandAlias::from_xml(cmd, dom.arena);
                std::cout << "- " << ca.name << std::endl;
                this->command_aliases.emplace(ca.name, std::move(ca));
                continue;
            }

            Command c = Command::from_xml(cmd, dom.arena);
            std::cout << "- " << c.name << std::endl;
            this->commands.emplace(c.name, std::move(c));
        }
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

    file << LineComment{ e.comment };
    file << "using " << e.name << Deprecate{ e.deprecated } << "= " << e.alias << ";\n\n";
}

// TASK: 030226_01, Also AI function
void sort_enum_items(std::vector<TypeEnum::EnumItem>& items) {
    // STEP 1: Move all concrete definitions (non-aliases) to the front.
    // std::stable_partition maintains the relative order of the concrete values
    // (preserving your bit-order logic like 1<<25, 1<<26 etc.)
    auto alias_start = std::stable_partition(items.begin(), items.end(),
        [](const TypeEnum::EnumItem& item) {
            return !item.is_alias;
        });

    // STEP 2: Resolve alias chains (e.g., A = B, B = C).
    // We need 'C' then 'B' then 'A'.
    // We iterate through the alias section and bubble up dependencies.

    bool changed = true;
    int max_passes = items.size(); // Safety break for circular dependencies

    while (changed && max_passes-- > 0) {
        changed = false;

        // Iterate only through the items identified as aliases
        for (auto it = alias_start; it != items.end(); ++it) {

            // Look ahead to see if the item this alias points to
            // is currently sitting *below* us in the list.
            auto dependency_it = std::find_if(std::next(it), items.end(),
                [&](const TypeEnum::EnumItem& potential_dep) {
                    return potential_dep.name == it->alias;
                });

            if (dependency_it != items.end()) {
                // We found the dependency lower down. Move it to right before us.
                // std::rotate effectively pulls dependency_it up to position it
                std::rotate(it, dependency_it, dependency_it + 1);

                // Since we shifted the list, mark as changed to re-scan
                // (in case the item we just moved up relies on something else)
                changed = true;
            }
        }
    }
}

void Generator::generate_enum(TypeEnum& e, std::ofstream& file) {
    file << LineComment{ e.comment };

    if (e.type == TypeEnum::Type::Constants) {
        for (auto& item : e.items) {
            file << "constexpr " << item.constant.type << " " << item.name << " = " << item.constant.value << ";"
                << LineComment{ item.comment, false } << '\n';
        }
        file << '\n';
        return;
    }
    // FIXME: this should be congigurable by config file, TASK: 100126_01
    file << "enum" << (0 ? " class" : "") << Deprecate{ e.deprecated } << e.name << " " << bitwidth_to_str_type(e.bitwidth) << " {\n";

    // TASK: 030226_01 Enum member dependencies
    sort_enum_items(e.items);

    for (TypeEnum::EnumItem& item : e.items) {
        if (item.is_standalone_comment) {
            file << "/* " << item.comment << " */\n";
            continue;
        }

        if (!item.api.empty() && !std::ranges::contains(std::views::split(item.api, ','), "vulkan", [](auto&& rng) { return sv(rng); }))
            continue;

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
            throw my_error("TODO: internal error");
            assert(false);
        }

        file << "," << LineComment{ item.comment, false } << '\n';
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
        // TODO: custom split
        if (!member.api.empty() && !std::ranges::contains(std::views::split(member.api, ','), "vulkan", [](auto&& rng) { return sv(rng); })) {
            std::cout << " TODO: Skipping member '" << member.stringify << "' of struct '" << s.name << "', because it does not contain 'vulkan' api.\n";
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
        // TODO: custom split
        if (!member.api.empty() && !std::ranges::contains(std::views::split(member.api, ','), "vulkan", [](auto&& rng) { return sv(rng); })) {
            std::cout << " TODO: Skipping member '" << member.stringify << "' of union '" << s.name << "', because it does not contain 'vulkan' api.\n";
            continue;
        }

        file << "    " << member.stringify << LineComment{ member.comment, false } << '\n';
    }
    file << "};\n\n";
}

void Generator::generate_bitmask(Type& bitmask, std::ofstream& file) {
    file << LineComment{ bitmask.comment };

    file << "using " << bitmask.name << Deprecate{ bitmask.deprecated } << "= ";
    if (!bitmask.alias.empty()) {
        file << bitmask.alias << ";\n";

    } else {
        // CHECK: revisit this
        auto it = std::ranges::find_if(bitmask.elem.children, has_tag("type"));
        if (it == bitmask.elem.children.end())
            throw my_error{ "bitmask type not found" };

        file << (*it)->asElement().children[0]->asText() << ";\n"; // TODO: check children
        std::cout << "bitmask: " << bitmask.name << " = " << (*it)->asElement().children[0]->asText() << std::endl;
    }

};



void Generator::generate_handle(Type& h, std::ofstream& file, TypeEnum& obj_enum) {
    assert(h.category == Type::Category::Handle);

    auto it = std::ranges::find(obj_enum.items, h.handle->objtypeenum, &TypeEnum::EnumItem::name);
    if (it == obj_enum.items.end())
        std::cout << "FIXME: Handle '" << h.name << "' did not found matching objtypeenum '" << h.handle->objtypeenum << "' in enum '" << obj_enum.name << "'" << std::endl;
    //throw my_error{ std::format("Handle '{}' did not found matching objtypeenum '{}' in enum '{}'", h.name, h.handle->objtypeenum, obj_enum.name) };

    if (true /*FIXME: OLD C HANDLES*/)
        file << "VK_DEFINE_HANDLE(" << h.name << ")\n";
    else
        file << "using " << h.name << Deprecate{ h.deprecated } << "= "
        << "Handle<struct " << obj_enum.name << "_T*>" << ";" << LineComment{ h.comment, false } << '\n';
}

std::ofstream& vkg_gen::Generator::Generator::generate_command_params(Command& cmd, std::ofstream& file, bool is_end_of_line) {
    file << "(";
    bool first = true;
    for (auto& param : cmd.parameters) {
        if (!param.api.empty() && !std::ranges::contains(std::views::split(param.api, ','), "vulkan", [](auto&& rng) { return sv(rng); })) {
            std::cout << " TODO: Skipping parameter '" << param.stringify << "' of command '" << cmd.name << "', because it does not contain 'vulkan' api.\n";
            continue;
        }

        if (first)
            first = false;
        else
            file << ", ";

        file << param.stringify;
    }

    file << ")";
    if (is_end_of_line)
        file << ";\n";
    return file;
}

void Generator::generate_command(Command& cmd, std::ofstream& file) {
    file << LineComment{ cmd.comment };
    // FIXME: cmd can have any arbitraty C code
    file << "VKAPI_ATTR " << cmd.type << " VKAPI_CALL " << cmd.name;
    generate_command_params(cmd, file);

}

void vkg_gen::Generator::Generator::generate_command_PFN(Command& cmd, std::ofstream& file) {
    // FIXME: cmd can have any arbitraty C code
    file << "typedef " << cmd.type << " (VKAPI_PTR* PFN_" << cmd.name << ")";
    generate_command_params(cmd, file);
}

void vkg_gen::Generator::Generator::generate_command_wrapper(Command& cmd, std::ofstream& file) {
    file << "inline " << cmd.type << " " << cmd.name; // TODO: proper name mangling
    generate_command_params(cmd, file, false);
    file << "{ return funcs." << cmd.name << "(";

    bool first = true;
    for (auto& param : cmd.parameters) {
        if (!param.api.empty() && !std::ranges::contains(std::views::split(param.api, ','), "vulkan", [](auto&& rng) { return sv(rng); })) {
            std::cout << " TODO: Skipping parameter '" << param.stringify << "' of command '" << cmd.name << "', because it does not contain 'vulkan' api.\n";
            continue;
        }

        if (first)
            first = false;
        else
            file << ", ";

        file << param.name;
    };
    file << "); }\n";

}

void Generator::add_required_type(sv name) {
    std::vector<Type*> required_types;
    add_required_type(name, required_types);
    assert(required_types.size() == 0);
}

void Generator::add_required_type(sv name, std::vector<Type*>& types_stack) {
    if (required_types.contains(name))
        return;

    if (!types.contains(name))
        std::cout << "FIXME: Required type not found: " << name << "\n";

    Type* type = &types.find(name)->second;
    if (std::ranges::find(types_stack, type) == types_stack.end())
        types_stack.push_back(type);
    else {
        std::cout << "FIXME: Returning from circular dependency: " << name << "\n";
        return;
    }


    // TODO: Write own split
    for (auto&& part : type->requires_ | std::views::split(',')) {
        sv req(&*part.begin(), std::ranges::distance(part));
        if (req.empty()) {
            std::cout << "FIXME: empty requires NOT HANDLED" << std::endl;
            break;
        }

        // FIXME: Circular dependency
        add_required_type(req, types_stack);


    }

    // CHECK: Im not sure about this logic. What if we have struct, that is defined and want to also be accessed by this alias?
    if (!type->alias.empty())
        add_required_type(type->alias, types_stack);
    else
        switch (type->category) {

            /// These types do not require any special handling
        case Type::Category::Basetype: // fallthrough
        case Type::Category::Bitmask:  // fallthrough
        case Type::Category::Define:   // fallthrough
            // CHECK: handle too?
        case Type::Category::Handle:   // fallthrough
            // TODO: currently generates all handles
        case Type::Category::Include:  // fallthrough
        case Type::Category::None:
            break;

        case Type::Category::Enum:
            add_required_enum(type->name);
            break;

        case Type::Category::Struct:   // fallthrough
        case Type::Category::Union:
            for (auto& member : type->struct_->members) {
                if (member.is_standalone_comment)
                    continue;

                add_required_type(member.type, types_stack);
            }
            break;

        case Type::Category::Funcpointer:
            constexpr auto asElementGetFirstChildAsTextTransform = std::views::transform([](Node* n) -> Node::Text& { return n->asElement().children[0]->asText(); });
            for (sv& param_type : type->elem.children | std::views::filter(has_tag("type")) | asElementGetFirstChildAsTextTransform)
                add_required_type(param_type, types_stack);

            for (sv& param_enum : type->elem.children | std::views::filter(has_tag("enum")) | asElementGetFirstChildAsTextTransform)
                add_required_type(param_enum, types_stack);

            break;

        }

    types_stack.pop_back();
    required_types.add_without_check(type);
}

void vkg_gen::Generator::Generator::add_required_enum(sv name) {
    // FIXME: API constatnts
    static const std::unordered_set<sv> api_constants = { "VK_MAX_PHYSICAL_DEVICE_NAME_SIZE", "VK_UUID_SIZE", "VK_LUID_SIZE", "VK_MAX_EXTENSION_NAME_SIZE", "VK_MAX_DESCRIPTION_SIZE", "VK_MAX_MEMORY_TYPES", "VK_MAX_MEMORY_HEAPS", "VK_LOD_CLAMP_NONE", "VK_REMAINING_MIP_LEVELS", "VK_REMAINING_ARRAY_LAYERS", "VK_REMAINING_3D_SLICES_EXT", "VK_WHOLE_SIZE", "VK_ATTACHMENT_UNUSED", "VK_TRUE", "VK_FALSE", "VK_QUEUE_FAMILY_IGNORED", "VK_QUEUE_FAMILY_EXTERNAL", "VK_QUEUE_FAMILY_FOREIGN_EXT", "VK_SUBPASS_EXTERNAL", "VK_MAX_DEVICE_GROUP_SIZE", "VK_MAX_DRIVER_NAME_SIZE", "VK_MAX_DRIVER_INFO_SIZE", "VK_SHADER_UNUSED_KHR", "VK_MAX_GLOBAL_PRIORITY_SIZE", "VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT", "VK_MAX_PIPELINE_BINARY_KEY_SIZE_KHR", "VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR", "VK_MAX_VIDEO_VP9_REFERENCES_PER_FRAME_KHR", "VK_SHADER_INDEX_UNUSED_AMDX", "VK_PARTITIONED_ACCELERATION_STRUCTURE_PARTITION_INDEX_GLOBAL_NV", "VK_MAX_PHYSICAL_DEVICE_DATA_GRAPH_OPERATION_SET_NAME_SIZE_ARM" };

    if (api_constants.contains(name))
        name = "API Constants";

    if (required_enums.contains(name) || required_types.contains(name))
        return;

    auto it = enums.find(name);
    if (it != enums.end()) {
        required_enums.add_without_check(&it->second);
        return;
    };

    auto it2 = types.find(name);
    if (it2 != types.end()) {
        required_types.add_without_check(&it2->second);
        add_required_enum(it2->second.alias);
        return;
    }

    // FIXME: New enum alias can be defined in extension require tag and then other extension can depend on it without redefining it
    // Also we're currently adding enum aliases ound in require tags into required_defines
    if (std::ranges::find(required_defines, name, &DefineExt::name) == required_defines.end())
        throw my_error{ std::format("Enum '{}' not found", name) };

}



void Generator::add_required_command(sv name) {
    // TASK: 040226_01 non-linkable commands
    static const std::unordered_set<sv> non_linkable_commands = { "vkGetRayTracingShaderGroupsKHR", "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", "vkGetRayTracingShaderGroupHandlesKHR", "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR", "vkGetCalibratedTimestampsKHR", "vkReleaseSwapchainImagesKHR" };
    if (non_linkable_commands.contains(name))
        return;

    if (required_commands.contains(name) || required_commands_aliases.contains(name))
        return;

    auto it = commands.find(name);
    if (it != commands.end()) {
        Command* cmd = &(it->second);
        required_commands.add_without_check(cmd);

        // return type
        add_required_type(cmd->type);

        for (auto& param : cmd->parameters)
            add_required_type(param.type);

    } else {
        CommandAlias* ca = &command_aliases.find(name)->second;

        // TASK: 040226_01 non-linkable commands
        if (non_linkable_commands.contains(ca->alias))
            return;

        required_commands_aliases.add_without_check(ca);

        add_required_command(ca->alias);
    }
}

void Generator::add_required_version_feature(sv name, vkg_gen::xml::Dom& dom) {
    // TODO: some feature map is needed
    auto it = std::ranges::find_if(dom.root->asElement().children, has_tag("feature") && has_attr("name", name));
    if (it == dom.root->asElement().children.end()) {
        throw my_error{ std::format("Feature (version) '{}' not found", name) };
    }
    Element& feature = (*it)->asElement();

    // TODO: check arguments of the feature.
    // attributes:
    // api - comma separated list,
    // apitype - values: 'internal' or 'public' <- default
    // name: we wont protect the feature, because we will generate specific features
    // number: major.minor
    // depends: 😭 String containing a boolean expression of one or more API core version and extension names. The feature requires the expression in the string to be satisfied to use any functionality it defines. Supported operators include , for logical OR, ` for logical AND, and `(` `)` for grouping. `,` and ` are of equal precedence, and lower than ( ). Expressions must be evaluated left-to-right for operators of the same precedence. Terms which are core version names are true if the corresponding API version is supported. Terms which are extension names are true if the corresponding extension is enabled.
    // sortorder
    // comment

    sv depends = feature.get_attr_value("depends");

    if (!depends.empty() && (depends.contains(',') || depends.contains('+'))) {
        std::cout << "FIXME: Compound dependencies not supported yet" << std::endl;
    } else if (!depends.empty()) {
        add_required_version_feature(depends, dom);
    }

    for (Node* node : feature.children) {
        if (node->isText()) {
            std::cout << "- Skipping: " << node->asText() << std::endl;
            continue;
        }
        auto& elem = node->asElement();
        if (elem.tag == "require") {
            // FIXME: handle arguments: depends, profile, api
            for (Node* type : elem.children) {
                if (type->isText()) {
                    std::cout << "- Skipping TEXT: " << type->asText() << std::endl;
                    continue;
                }

                Element& elem = type->asElement();

                if (elem.tag == "type") {
                    add_required_type(elem.get_attr_value("name"));
                } else if (elem.tag == "enum") {
                    auto it = std::ranges::find(elem.attrs, "extends", &Attribute::name);
                    if (it != elem.attrs.end()) {
                        // TASK: 040226_02 Extension number
                        extend_enum(it->value, elem, dom.arena);
                        add_required_enum(it->value);
                    } else {
                        add_required_enum(elem.get_attr_value("name"));
                    }


                } else if (elem.tag == "command") {
                    add_required_command(elem.get_attr_value("name"));

                } else {
                    std::cout << "- FIXME: currently ignoring: ";
                    helper_test(std::cout, type) << std::endl;
                }
            }
        } else if (elem.tag == "deprecate") {
            // TODO: handle profile and api attributes
            sv explanation = elem.get_attr_value("explanationlink");

            for (Node* type : elem.children) {
                if (type->isText()) {
                    std::cout << "- Skipping TEXT: " << type->asText() << std::endl;
                    continue;
                }

                Element& elem = type->asElement();

                if (elem.tag == "type") {
                    sv name = elem.get_attr_value("name");
                    types.find(name)->second.deprecated = explanation;
                } else if (elem.tag == "enum") {
                    // TODO: enum can have also api attribute
                    sv name = elem.get_attr_value("name");
                    enums.find(name)->second.deprecated = explanation;
                } else if (elem.tag == "command") {
                    // TODO: Currently ignoring commands
                    //sv name = elem.get_attr_value("name");
                    //commands.find(name)->second.deprecated = explanation;
                } else if (elem.tag == "comment") {
                    // CHECK: ignore standalone comments?
                } else {
                    throw my_error{ "Unknown tag '" + std::string(elem.tag) + "' in deprecate tag." };
                }
            }

        } else if (elem.tag == "remove") {
            // TODO: handle profile, api, reasonlink and comment attributes
            for (Node* type : elem.children) {
                if (type->isText()) {
                    std::cout << "- Skipping TEXT: " << type->asText() << std::endl;
                    continue;
                }

                Element& elem = type->asElement();

                if (elem.tag == "type") {
                    required_types.remove(elem.get_attr_value("name"));
                } else if (elem.tag == "enum") {
                    required_enums.remove(elem.get_attr_value("name"));
                } else if (elem.tag == "command") {
                    required_commands.remove(elem.get_attr_value("name"));
                } else if (elem.tag == "comment") {
                    // CHECK: ignore standalone comments?
                } else {
                    throw my_error{ "Unknown tag '" + std::string(elem.tag) + "' in remove tag." };
                }
            }

        } else {
            std::cout << "- TODO: skipping: " << elem.tag << std::endl;
        }

    }
}

void Generator::extend_enum(sv extends, Element& elem, vkg_gen::Arena& arena, sv block_ext_number) {

    TypeEnum& enum_ = enums.find(extends)->second;
    auto item = TypeEnum::EnumItem::from_xml(elem, arena, &enum_, false, true, block_ext_number);

    // TASK: 030226_01: Enum member dependencies/redundancy
    if (std::ranges::find(enum_.items, item.name, &TypeEnum::EnumItem::name) != enum_.items.end())
        std::cout << "- WARNING: duplicate enum item: '" << item.name << "' in enum '" << extends << "\n";
    else
        enum_.items.emplace_back(std::move(item));
}

// TASK: 100126_01
void Generator::add_extension_prototype(sv number, xml::Dom& dom) {
    auto& registry = dom.root->asElement();
    assert(registry.tag == "registry");


    Node* extensions = (registry.children | std::views::filter(has_tag("extensions"))).front();
    if (extensions == nullptr)
        throw my_error{ "Could not find extensions tag" };

#if 0
    Node* extension = (extensions->asElement().children | std::views::filter(has_attr("number", number))).front();
    if (extension == nullptr)
        throw my_error{ "Could not find extension with number " + std::string(number) };
#endif

    for (Node* extension : extensions->asElement().children) {
        if (extension->isText() || extension->asElement().tag != "extension")
            continue;

        auto& ext = extension->asElement();

        sv supported = ext.get_attr_value("supported");
        if (supported == "disabled" || !std::ranges::contains(std::views::split(supported, ','), "vulkan", [](auto&& rng) { return sv(rng); }))
            continue;

        if (!ext.get_attr_value("platform").empty())
            continue;

        included_extensions.push_back(ext);

        for (Node* node : ext.children) {
            if (node->isText()) {
                std::cout << "- Skipping: " << node->asText() << std::endl;
                continue;
            }

            auto& elem = node->asElement();
            if (elem.tag == "require") {
                // FIXME:
                if (!elem.get_attr_value("platform").empty())
                    continue;

                for (Node* type : elem.children) {
                    if (type->isText()) {
                        std::cout << "- Skipping TEXT: " << type->asText() << std::endl;
                        continue;
                    }

                    Element& elem = type->asElement();

                    if (elem.tag == "type") {
                        add_required_type(elem.get_attr_value("name"));
                    } else if (elem.tag == "enum") {
                        auto it = std::ranges::find(elem.attrs, "extends", &Attribute::name);
                        if (it != elem.attrs.end()) {
                            // TASK: 040226_02 Extension number
                            extend_enum(it->value, elem, dom.arena, number);
                            add_required_enum(it->value);
                            continue;
                        }

                        sv name = elem.get_attr_value("name");
                        it = std::ranges::find(elem.attrs, "value", &Attribute::name);
                        if (it != elem.attrs.end()) {
                            required_defines.push_back({ name, it->value, elem });
                            continue;
                        }

                        it = std::ranges::find(elem.attrs, "alias", &Attribute::name);
                        if (it != elem.attrs.end()) {
                            // FIXME: should be enum alias
                            required_defines.push_back({ name, it->value, elem });
                            continue;
                        }

                        add_required_enum(name);


                    } else if (elem.tag == "command") {
                        add_required_command(elem.get_attr_value("name"));

                    } else {
                        std::cout << "- FIXME: currently ignoring: ";
                        helper_test(std::cout, type) << std::endl;
                        continue;
                    }
                }
            }
        }
    }
}


void Generator::generate(xml::Dom& dom, std::ofstream& header, std::ofstream& source, Config& config) {


#if 1
    header << "#pragma once\n";
    header << boilerplate::VIDEO_INCLUDES << "\n";

    header << "#define VKAPI_PTR\n";

    header << "typedef uint32_t VkFlags;\n";
    header << "typedef uint64_t VkFlags64;\n";

    header << boilerplate::HANDLE_DEFINITION << "\n";

    header << "#define VKAPI_CALL\n";
    header << "#define VKAPI_ATTR\n";
    header << "#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;\n";

    //generate_API_constants(dom, file);
    //generate_types(dom, file);

    parse_types(dom);
    parse_enums(dom);
    parse_commands(dom);

    add_required_version_feature("VK_VERSION_1_4", dom);

    add_extension_prototype("1", dom);

    // Currently generating all enums
    for (TypeEnum* enum_ : required_enums.get()) {
        if (enum_ != nullptr)
            generate_enum(*enum_, header);
    }

    // TASK: 100126_01
    for (auto& define : required_defines) {
        header << "#define " << define.name << " " << define.value << "\n";
    }

    for (Type* p : required_types.get()) {
        if (p == nullptr)
            continue;

        auto& type = *p;
        // TODO: this should not be needed
        if (exclude_platforms(type.name)) {
            std::cout << "Excluding: " << type.name << std::endl;
            continue;
        }

        switch (type.category) {
        case Type::Category::Struct:
            generate_struct(type, header);
            break;
        case Type::Category::Union:
            generate_union(type, header);
            break;
        case Type::Category::Enum:
            // TODO: this should not be needed and should not even happen
            generate_enum_alias(type, header);
            break;

        case Type::Category::Bitmask:
            generate_bitmask(type, header);
            break;

            // TODO: temporary solution
        case Type::Category::Basetype:
            //case Type::Category::Define:
        case Type::Category::Funcpointer:
            _gen_arbitrary_C_code_in_type(type.elem, header);
            break;
        case Type::Category::Handle:
            generate_handle(type, header, enums.at(TypeHandle::obj_enum_name));
            break;
        default:
            break;
        }
    }

    //header << "extern \"C\" {\n";
    //for (Command* cmd : required_commands.get()) {
    //    if (cmd == nullptr)
    //        continue;
    //    generate_command(*cmd, header);
    //}
    //header << "}\n\n";

    for (Command* cmd : required_commands.get()) {
        if (cmd == nullptr)
            continue;
        generate_command_PFN(*cmd, header);
    }

    header << "\n";
    header << "struct FuncTable {\n";

    for (Command* cmd : required_commands.get()) {
        if (cmd == nullptr)
            continue;

        header << "    PFN_" << cmd->name << " " << cmd->name << " = nullptr;\n";
    }
    header << "};\nextern FuncTable funcs;\n";


    for (CommandAlias* ca : required_commands_aliases.get()) {
        if (ca == nullptr)
            continue;
        header << "using " << "PFN_" << ca->name << " = PFN_" << ca->alias << ";\n";
        // FIXME: because of removing the static linking, the ca->alias is not defined
        // header << "auto " << ca->name << " = " << ca->alias << ";\n";
    }

    for (Command* cmd : required_commands.get()) {
        if (cmd == nullptr)
            continue;

        generate_command_wrapper(*cmd, header);
    }

    // FIXME: causes redefinition
    //for (CommandAlias* ca : required_commands_aliases.get()) {
    //    if (ca == nullptr)
    //        continue;
    //    header << "auto " << ca->name << " = " << ca->alias << ";\n"; // TODO: proper name mangling
    //}

    header << "\n" << boilerplate::HEADER_EXTERNS;

    source << "#include \"" << config.header_path << "\"\n";
    source << boilerplate::CPP_IMPL;
    source << "// static const char* extensions[] = {";
    bool first = true;
    for (const Element& extension : included_extensions) {
        sv name = extension.get_attr_value("name");
        //if (exclude_platforms(name))
        //    continue;
        if (extension.get_attr_value("type") != "instance")
            continue;

        if (first)
            first = false;
        else
            source << ", ";

        source << "\"" << name << "\"";
    }
    source << "};\n\n";

    source << boilerplate::LOAD_UNLOAD_LIB_IMPL << "\n";

    static const std::unordered_set<std::string_view> ignore = {
        "vkGetInstanceProcAddr",
        "vkCreateInstance",
        "vkEnumerateInstanceExtensionProperties",
        "vkEnumerateInstanceLayerProperties",

        "vkGetPhysicalDeviceExternalImageFormatPropertiesNV", // TODO: this crashes the program
    };

    source << "void init_vk_functions() {\n";
    for (Command* cmd : required_commands.get()) {

        if (cmd == nullptr || ignore.contains(cmd->name))
            continue;

        source << "    funcs." << cmd->name << " = (PFN_" << cmd->name << ")funcs.vkGetInstanceProcAddr(instance, \"" << cmd->name << "\");\n";
    }

    for (Command* cmd : required_commands.get()) {

        if (cmd == nullptr || ignore.contains(cmd->name))
            continue;

        source << "    assert(funcs." << cmd->name << ");\n";
    }
    source << "}\n";

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

    //auto extension_filter = has_tag("extension");
    //auto extensions = dom.getChildrenByTag("extensions")[0]->asElement();
    //for (Node* child : extensions.children | std::views::filter(extension_filter)) {
    //    auto& ch = child->asElement();
    //    std::cout << ch.get_attr_value("name") << " = " << ch.get_attr_value("number") << ",\n";
    //}
    using Element = vkg_gen::xml::Element;

    //auto asElementConstTransform = std::views::transform([](const Node* n) ->const Element& { return n->asElement(); });
    auto feature_filter = has_tag("feature") && [](const Node* n) { return n->isElement() && n->asElement().get_attr_value("name").contains("VK_VERSION"); };


    auto asElementTransform = std::views::transform([](Node* n) -> Element& { return n->asElement(); });
    auto it = std::ranges::find_if(dom.root->asElement().children, has_tag("extensions"));
    Element& extensionsTag = (*it)->asElement();

    //parse_enums(dom);

    for (Element& extension : extensionsTag.children | std::views::filter(has_tag("extension")) | asElementTransform) {
        for (Element& require : extension.children | std::views::filter(has_tag("require")) | asElementTransform) {
            for (Element& enum_ : require.children | std::views::filter(has_tag("enum") && has_attr_name("offset") && !has_attr_name("extnumber")) | asElementTransform) {
                std::cout << "Found enum extension with offset and without extnumber: " << "<enum ";
                for (auto& attr : enum_.attrs) {
                    std::cout << attr.name << "='" << attr.value << "', ";
                }
                std::cout << ">\n";
            }

            //for (Element& enum_ : require.children | std::views::filter(has_tag("enum") && has_attr_name("offset")) | asElementTransform) {
            //    TypeEnum& parent = enums.find(enum_.get_attr_value("extends"))->second;
            //    if (parent.type != TypeEnum::Type::Normal) {
            //        std::cout << "Found enum extension with offset on enum(" << to_string(parent.type) << "): " << "<enum ";
            //        for (auto& attr : enum_.attrs) {
            //            std::cout << attr.name << "='" << attr.value << "', ";
            //        }
            //        std::cout << ">\n";
//
            //    }
            //}
        }
    }
#endif
};

CommandParameter CommandParameter::from_xml(const xml::Element& elem, vkg_gen::Arena& arena) {
    assert(elem.tag == "param");

    CommandParameter param;

    for (const Attribute& attr : elem.attrs) {
        if (attr.name == "type")
            param.type = attr.value;
        else if (attr.name == "api")
            param.api = attr.value;
        else if (attr.name == "len")
            param.len = attr.value;
        else if (attr.name == "altlen")
            param.altlen = attr.value;
        else if (attr.name == "stride")
            param.stride = attr.value;
        else if (attr.name == "optional")
            param.optional = attr.value;
        else if (attr.name == "selector")
            param.selector = attr.value;
        else if (attr.name == "noautovalidity")
            param.noautovalidity = attr.value;
        else if (attr.name == "externsync")
            param.externsync = attr.value;
        else if (attr.name == "objecttype")
            param.objecttype = attr.value;
        else if (attr.name == "validstructs")
            param.validstructs = attr.value;
        else
            throw my_error{ "Unknown attribute for command parameter: " + std::string(attr.name) };
    };
    for (Node* child : elem.children) {
        if (child->isText()) {
            param.stringify += child->asText();
            continue;
        }

        auto& ch = child->asElement();
        if (ch.tag == "name") {

            if (!param.name.empty())
                throw my_error{ "Command parameter can only have one name!" };
            param.name = ch.children[0]->asText();
            param.stringify += " ";
            param.stringify += param.name;

        } else if (ch.tag == "type") {
            if (param.type.empty()) {
                param.type = ch.children[0]->asText();
            } else {
                // TASK: 090126_08 - THIS CAN HAPPEN
                std::cout << "TODO: Duplicate type in member: " << std::string(param.name) << std::endl;
            }
            param.stringify += ch.children[0]->asText();
        } else {
            throw my_error{ "Unknown child element for command parameter: " + std::string(ch.tag) };
        }
    }
    if (param.name.empty())
        throw my_error{ "Command parameter must have a name!" };

    // CHECK: in vulkan spec it's said to be optional
    if (param.type.empty())
        throw my_error{ "Command parameter must have a type!" };
    return param;
}

Command Command::from_xml(const xml::Element& elem, vkg_gen::Arena& arena) {
    Command cmd;

    for (const Attribute& attr : elem.attrs) {
        if (attr.name == "tasks")
            cmd.tasks = attr.value;
        else if (attr.name == "queues")
            cmd.queues = attr.value;
        else if (attr.name == "successcodes")
            cmd.success_codes = attr.value;
        else if (attr.name == "errorcodes")
            cmd.error_codes = attr.value;
        else if (attr.name == "renderpass")
            cmd.render_pass = scope_from_string(attr.value);
        else if (attr.name == "videocoding")
            cmd.video_encoding = scope_from_string(attr.value);
        else if (attr.name == "cmdbufferlevel")
            cmd.cmd_buffer_level = attr.value;
        else if (attr.name == "conditionalrendering")
            cmd.conditional_rendering = attr.value;
        else if (attr.name == "allownoqueues")
            cmd.allow_no_queues = bool_from_string(attr.value); // TODO: no ',' allowed
        else if (attr.name == "export")
            cmd.export_ = attr.value;
        else if (attr.name == "api")
            cmd.api = attr.value;
        else if (attr.name == "comment")
            cmd.comment = attr.value;
        else if (attr.name == "alias" || attr.name == "name")
            throw my_error{ "Command attribute '" + std::string(attr.name) + "' is not supported" };
        else {
            std::cerr << elem << std::endl;
            throw my_error{ "Unknown attribute for command: " + std::string(attr.name) };

        }
    };

    if (elem.children.size() == 0)
        throw my_error{ "Command must have at least <proto> element!" };

    Node* proto = elem.children[0];
    if (proto->isText())
        throw my_error{ "Command must start with <proto> element!" };

    if (proto->asElement().tag != "proto")
        throw my_error{ "Command must start with <proto> element!" };

    for (Node* child : proto->asElement().children) {
        if (child->isText()) {

            cmd.declatarion += child->asText();
            continue;
        }
        auto& ch = child->asElement();
        if (ch.tag == "name") {
            if (!cmd.name.empty())
                throw my_error{ "Command can only have one name!" };
            cmd.name = ch.children[0]->asText();
            cmd.declatarion += cmd.name;
        } else if (ch.tag == "type") {
            if (cmd.type.empty()) {
                cmd.type = ch.children[0]->asText();
            } else {
                // TASK: 090126_08 - THIS CAN HAPPEN
                std::cout << "TODO: Duplicate type in member: " << std::string(cmd.name) << std::endl;
            }
            cmd.declatarion += ch.children[0]->asText();
        } else
            throw my_error{ "Unknown child element for command: " + std::string(child->asElement().tag) };
    }

    if (cmd.name.empty())
        throw my_error{ "Command must have a name!" };

    // CHECK: in vulkan spec it is said to be optional
    if (cmd.type.empty())
        throw my_error{ "Command must have a type!" };

    for (Node* child : elem.children | std::views::drop(1)) {
        if (child->isText()) {
            std::cout << "- TASK: 090126_04 - Skipping TEXT: " << child->asText() << std::endl;
            continue;
        }
        auto& ch = child->asElement();

        if (ch.tag == "param")
            cmd.parameters.emplace_back(CommandParameter::from_xml(child->asElement(), arena));
        else if (ch.tag == "implicitexternsyncparams")
            cmd.implicit_extern_sync_params = &child->asElement();
        else if (ch.tag == "description")
            cmd.description = ch.children[0]->asText();
        else
            throw my_error{ "Unknown child element for command: " + std::string(child->asElement().tag) };

    }

    return cmd;
}

CommandAlias vkg_gen::Generator::CommandAlias::from_xml(const xml::Element& elem, vkg_gen::Arena& arena) {
    CommandAlias ca;

    for (const Attribute& attr : elem.attrs) {
        if (attr.name == "name")
            ca.name = attr.value;
        else if (attr.name == "alias")
            ca.alias = attr.value;
        else if (attr.name == "comment")
            ca.comment = attr.value;
        else if (attr.name == "api")
            ca.api = attr.value;
        else {
            std::cerr << elem << std::endl;
            throw my_error{ "Unknown attribute for command: " + std::string(attr.name) };
        }
    };

    if (elem.children.size() != 0)
        throw my_error{ "Command alias must have no children!" };

    if (ca.name.empty())
        throw my_error{ "Command alias must have a name!" };

    assert(!ca.alias.empty());

    return ca;
}

