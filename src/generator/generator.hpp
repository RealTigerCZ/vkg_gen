/**
 * @file generator.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief
 * @date Created: 02. 11. 2025
 * @date Modified: 2. 11. 2025
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "../xml/xml.hpp"
#include <map>

constexpr int BITPOS_MAX = 64 - 1;

namespace vkg_gen::Generator {
    using sv = std::string_view;


    struct Generator {
        // <platforms comment="...">,
        // <platform name="..." protect="..." comment="..."/>
        // </platforms>
        xml::Node* Platforms;

        // <tags comment="...">
        // <tag name="..." author="..." contact="..."/>
        // </tags>
        xml::Node* Tags;

        // <types comment="...">
        // <type ... > ... </type>
        // <comment ... > ... </comment>
        // </types>
        xml::Node* Types;
        xml::Node* Enums;
        xml::Node* Commands;
        xml::Node* Feature;
        xml::Node* Extensions;
        xml::Node* Formats;
        xml::Node* Sync;
        xml::Node* VideoCodecs;
        xml::Node* SpirvExtensions;
        xml::Node* SpirvCapabilities;
    };

    // <platform name="..." protect="..." comment="...">
    struct Platform {
        sv name;
        sv protect;
        sv comment;
    };

    // <tag name="..." author="..." contact="...">
    struct Tag {
        sv name;
        sv author;
        sv contact;
    };

    // <type category="define" requires="..." comment="..." api="..." name="...">
    // arbitrary C code with <name>...</name>, <type>...</type>
    // </type>
    struct TypeDefine {
        sv _requires;
        sv comment;
        sv api;
        sv name;
        xml::Node* node;
    };

    // <type category="include" name="...">
    // arbitrary C code
    // </type>
    struct TypeInclude {
        sv name;
        sv value;
    };

    // <type category="basetype">
    // arbitrary C code with <name>...</name>, <type>...</type>
    // </type>
    struct TypeBase {
        sv name;
        xml::Node* node;
    };

    /**
         struct Type {
         xml::Node* node;
        sv name;

    }
    */

    // <type category="bitmask" api="..." requires="..." name="..." alias="...">
    // arbitrary C code with <name>...</name>, <type>...</type>
    // </type>
    struct TypeBitmask {
        sv _requires;
        sv api;
        sv name;
        sv alias;
        xml::Node* node;
    };

    // <type category="handle" parent="..." objtypeenum="..." alias="...">
    // arbitrary C code with <name>...</name>, <type>...</type>
    // </type>
    struct TypeHandle {
        sv parent;
        sv objtypeenum;
        xml::Node* node;
    };

    // <type category="enum" name="..." alias="..."/>
    struct TypeEnumDecl {
        sv name;
        sv alias;
    };

    // <type category="funcpointer" requires="...">
    // arbitrary C code with <name>...</name>, <type>...</type>
    // </type>
    struct TypeFuncPtr {
        sv _requires;
        sv name;
        xml::Node* node;
    };

    // <member optional="true/false" noautovalidity="true/false" limittype="..." len="..." values="..." deprecated="...">
    // arbitrary C code with <name>...< / name>, <type>...< / type>, <enum>...< / enum>, <comment>...< / comment>
    //</ member>
    /*
    #   <member> - like <param> for a struct or union member
    #       len - if the member is an array, len may be one or more of the following
    #           things, separated by commas (one for each array indirection):
    #           another member of that struct, 'null-terminated' for a string,
    #           '1' to indicate it is just a pointer (used for nested pointers),
    #           or a latex equation (prefixed with 'latexmath:')
    #       altlen - if len has latexmath equations, this contains equivalent C99
    #                expressions separated by commas.
    #       stride - if the member is an array, stride specifies the name of
    #           another member containing the byte stride between consecutive
    #           elements in the array. The array is assumed to be tightly packed
    #           if omitted.
    #       deprecated - denotes that this member is deprecated, and why.
    #           Valid values: 'ignored', 'true'.
    #       externsync - denotes that the member should be externally synchronized
    #           when accessed by Vulkan
    #       optional - whether this value can be omitted by providing NULL (for
    #           pointers), VK_NULL_HANDLE (for handles) or 0 (for bitmasks/values)
    #       selector - for a union member, identifies a separate enum member that
    #           selects which of the union's members are valid
    #       selection - for a member of a union, identifies one or more commad-separated
    #           enum values indicating that member is valid
    #       noautovalidity - tag stating that no automatic validity language should be generated
    #       values - comma-separated list of legal values, usually used only for sType enums
    #       limittype - only applicable for members of VkPhysicalDeviceProperties and
    #           VkPhysicalDeviceProperties2, their substructures, and extensions.
    #           Specifies the type of a device limit.
    #           Valid values: 'min', 'max', 'not', 'pot', 'mul', 'bits', bitmask', 'range', 'struct', 'exact', 'noauto'
    #       objecttype - only applicable for members representing a handle as
    #           a uint64_t value. Specifies the name of another member which is
    #           a VkObjectType or VkDebugReportObjectTypeEXT value specifying
    #           the type of object the handle references.
    #       featurelink - only applicable for members representing a Boolean API
    #           feature. Specifies that the feature has a link in the
    #           specification that does not match the name of the feature.
    #           Typically for features in extensions that were later promoted but
    #           with changes.
    */
    struct TypeStructMember {
        bool optional;
        bool noautovalidity;
        sv limittype;
        sv len;
        sv values;
        sv deprecated;
        sv externsync;
        xml::Node* node;
    };

    // <type category="struct" requires="..." name="...">
    // <member optional="true/false"> arbitrary C code with <name>...</name>, <type>...</type> </member> any number of them
    // </type>
    struct TypeStruct {
        sv _requires;
        sv name;
        std::vector<TypeStructMember> members;
    };

    struct NameTable {
        std::map<sv, sv> _requires; // name 1 requires name 2
        std::map<sv, xml::Node*> provides; // name is provided by this node (in attribute or it is itself <name>...</name>)
        std::map<sv, sv> alias_resolution_table; // alias -> name
    };
} // namespace


struct TypeHandle {
    using sv = std::string_view;
    static constexpr sv obj_enum_name = "VkObjectType";

    sv parent; // Notes another type with the 'handle'
    // category that acts as a parent object for
    // this type.

    sv objtypeenum; // name of VK_OBJECT_TYPE_* API enumerant which
    // corresponds to this type. Should be present
};

struct Member {
    using sv = std::string_view;
    enum class LimitType : uint16_t {
        None = 0,
        Min = 1,
        Max = 1 << 1,
        Not = 1 << 2,
        Pot = 1 << 3,
        Mul = 1 << 4,
        Bits = 1 << 5,
        Bitmask = 1 << 6,
        Range = 1 << 7,
        Struct = 1 << 8,
        Exact = 1 << 9,
        NoAuto = 1 << 10
    };

    enum class ExternSync : uint16_t {
        False,
        True,
        Maybe
    };

    sv name;
    sv type;
    sv api;

    // if the member is an array, len may be one or more of the following
    // things, separated by commas (one for each array indirection):
    // another member of that struct, 'null-terminated' for a string,
    // '1' to indicate it is just a pointer (used for nested pointers),
    // or a latex equation (prefixed with 'latexmath:')
    sv len;

    // if len has latexmath equations, this contains equivalent C99
    // expressions separated by commas.
    sv altlen;

    // if the member is an array, stride specifies the name of
    // another member containing the byte stride between consecutive
    // elements in the array.The array is assumed to be tightly packed
    // if omitted.
    sv stride;
    sv deprecated; // denotes that this member is deprecated, and why

    // denotes that this member should be externally synchronized when accessed by Vulkan
    ExternSync externsync;
    bool optional; // denotes whether this value can be omitted by providing NULL

    // for a union member, identifies a separate enum member that
    // selects which of the union's members are valid
    sv selector;


    // for a member of a union, identifies one or more commad-separated
    // enum values indicating that member is valid
    sv selection;
    bool noautovalidity; // tag stating that no automatic validity language should be generated
    sv values; // omma-separated list of legal values, usually used only for sType enums

    // Only applicable for members of VkPhysicalDeviceProperties and
    // VkPhysicalDeviceProperties2, their substructures, and extensions.
    // Specifies the type of a device limit.
    LimitType limittype;

    // Only applicable for members representing a handle as
    // a uint64_t value. Specifies the name of another member which is
    // a VkObjectType or VkDebugReportObjectTypeEXT value specifying
    // the type of object the handle references.
    sv objecttype;

    // Only applicable for members representing a Boolean API
    // feature. Specifies that the feature has a link in the
    // specification that does not match the name of the feature.
    // Typically for features in extensions that were later promoted but
    // with changes.
    sv featurelink;

    const vkg_gen::xml::Element& element;

    sv comment;
    bool is_standalone_comment = false;
    std::string stringify;

    enum class ParentType : uint8_t {
        Struct,
        Union
    };

    Member(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena, ParentType parent_type, bool is_standalone_comment = false);

    LimitType limit_type_from_string(std::string_view s);
};


struct TypeStruct {
    using sv = std::string_view;

    bool returned_only = false; // Notes that this struct is going to be filled in
    // by the API, rather than an application filling
    // it out and passing it to the API.

    bool allow_duplicate = false; // pNext can include multiple structures of this type.

    sv struct_extends; // Lists parent structures which this structure may extend
    // via the pNext chain of the parent. When present it
    // suppresses generation of automatic validity for the
    // pNext member of that structure, and instead the
    // structure is added to pNext chain validity for the
    // parent structures it extends.

    std::vector<Member> members;
};

using TypeUnion = TypeStruct;

struct TypeEnum {
    using sv = std::string_view;

    struct ValueNormal {
        sv value;
    };

    struct ValueBitmask {
        bool is_bitfield = false;
        union {
            uint8_t bitpos = 0;
            sv value;
        };
    };

    struct ValueConstant {
        sv value;
        sv type;
    };


    struct EnumItem {
        sv name;
        sv comment;

        union {
            ValueNormal normal = {};
            ValueBitmask bitmask;
            ValueConstant constant;
            sv alias;
        };

        sv protect;
        sv deprecated;
        sv api;
        bool is_alias = false;
        bool is_standalone_comment = false; //TODO:

        EnumItem(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena, TypeEnum* parent, bool is_standalone_comment = false);
    };


    enum class Type : uint8_t {
        None,
        Bitmask,
        Normal,
        Constants
    };

    enum class Bitwidth : uint8_t {
        None,
        _8,
        _16,
        _32,
        _64
    };

    sv name;
    sv comment;
    Type type = Type::None;
    Bitwidth bitwidth = Bitwidth::None;
    // TODO: what about <unused>?
    std::vector<EnumItem> items;

    TypeEnum(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena);

private:
    Type type_from_string(sv s);
    Bitwidth bitwidth_from_string(sv s);

    void load_enum(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena);
};


class Type {
    using sv = std::string_view;
public:
    Type(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena);


    enum class State : uint8_t {
        NotUsed,
        Declared,
        Defined
    };

    enum class Category : uint8_t {
        Basetype,
        Bitmask,
        Define,
        Enum, // content should be empty, only declaration
        Handle,
        Funcpointer,
        Struct,
        Union,
        Include,
        None // content may include types <apientry />, <name>...</name>,
             // <type>...</type>, tag <type> may be multiple imbedded <type> tags
    };

    sv name; // name attribute or present it <name>...</name>, always must be present
    sv api; // api attribute, matches a <feature> api attribute, if present
    sv alias; // alias attribute
    sv requires_; // requires attribute, pointing to another type
    sv comment; // comment attribute
    sv deprecated; // reason for deprecation if present, can also have values "true" or "false"
    Category category = Category::None;
    State state = State::NotUsed;
    union {
        TypeHandle* handle;
        TypeStruct* struct_;
        TypeUnion* union_;
        sv bitvalues; // only for category Bitmask, name of an enum definition that defines the valid values for parameters of that type
    };


    const vkg_gen::xml::Element& elem;

private:
    Category category_from_string(std::string_view s) const;
    void parse_struct(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena);
    void parse_union(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena);
};

void generate_types(vkg_gen::xml::Dom& dom, std::ofstream& file);

void generate_base_types(const vkg_gen::xml::Dom& dom, std::ofstream& file);
void generate_enums(const vkg_gen::xml::Dom& dom, std::ofstream& file);
void generate_API_constants(const vkg_gen::xml::Dom& dom, std::ofstream& file);
bool bool_from_string(std::string_view s);

class Generator {
    using sv = std::string_view;
    std::map<sv, Type> types = {};
    std::map<sv, TypeEnum> enums = {};
    std::map<sv, Type*> enum_index = {};

    // FIXME: this is a hack, parse types should not need to use file
    void parse_types(vkg_gen::xml::Dom& dom, std::ofstream& file);
    void parse_enums(vkg_gen::xml::Dom& dom);

    void generate_enum(TypeEnum& enum_, std::ofstream& file);
    void generate_enum_alias(Type& enum_, std::ofstream& file);
    void generate_struct(Type& struct_, std::ofstream& file);
    void generate_union(Type& struct_, std::ofstream& file);
    void generate_bitmask(Type& bitmask, std::ofstream& file);
    void generate_handle(Type& handle, std::ofstream& file, TypeEnum& obj_enum);

public:
    Generator() {}
    void generate(vkg_gen::xml::Dom& dom, std::ofstream& file, void* config = nullptr);
};
