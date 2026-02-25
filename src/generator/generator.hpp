/**
 * @file generator.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief
 * @date Created: 02. 11. 2025
 * @date Modified: 25. 02. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

#include "../debug_macros.h"
#include "../config.hpp"
#include "../xml/xml.hpp"
#include <map>
#include <stack>
#include <unordered_map>
#include <unordered_set>

 // TASK: 090126_01
#define CONCEPT_FILTERING

namespace vkg_gen::Generator {
    // CHECK: vulkan spec
    constexpr int BITPOS_MAX = 64 - 1;

    // TASK: 090126_02
    using sv = std::string_view;

    // TODO: can we optimize size of optional attributes?
    struct Platform {
        sv name; // Expected short C99 identifier compatible name
        sv protect = {}; // C99 preprocessor token, starting with VK_USE_PLATFORM_
        sv comment = {};

        static Platform from_xml(const xml::Element& elem) { UNUSED(elem); NOT_IMPLEMENTED(); };
    };

    struct Platforms {
        sv comment = {};
        std::vector<Platform> platforms = {};

        static Platforms from_xml(const xml::Element& elem) { UNUSED(elem); NOT_IMPLEMENTED(); };
    };

    struct Tag {
        sv name;
        sv author;
        sv contact;

        static Tag from_xml(const xml::Element& elem) { UNUSED(elem); NOT_IMPLEMENTED(); };
    };

    struct Tags {
        sv comment = {};
        std::vector<Tag> tags = {};

        static Tags from_xml(const xml::Element& elem) { UNUSED(elem); NOT_IMPLEMENTED(); };
    };


    struct TypeHandle {
        // Name of the enum class containing VK_OBJECT_TYPE_*
        static constexpr sv obj_enum_name = "VkObjectType";

        sv parent; // Notes another type with the 'handle'
        // category that acts as a parent object for
        // this type.

        sv objtypeenum; // name of VK_OBJECT_TYPE_* API enumerant which
        // corresponds to this type. Should be present
    };

    struct Member {
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
        sv values; // comma-separated list of legal values, usually used only for sType enums

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

    // TODO: TypeUnion
    using TypeUnion = TypeStruct;

    struct TypeEnum {

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

            static EnumItem from_xml(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena, TypeEnum* parent, bool is_standalone_comment = false, bool extend_parent = false, sv block_ext_number = "");
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
        sv deprecated;
        Type type = Type::None;
        Bitwidth bitwidth = Bitwidth::None;
        // TODO: what about <unused>?
        std::vector<EnumItem> items;

        // TODO: change to static from_xml(...)
        TypeEnum(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena);

    private:
        Type type_from_string(sv s);
        Bitwidth bitwidth_from_string(sv s);
    };


    class Type {
    public:
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

        const xml::Element& elem;

        // TODO: change to static from_xml(...)
        Type(const xml::Element& elem, vkg_gen::Arena& arena);

    private:
        Category category_from_string(sv s) const;
        void parse_struct(const xml::Element& elem, Arena& arena);
        void parse_union(const xml::Element& elem, Arena& arena);
    };

    class CommandParameter {
    public:
        sv api = {}; // comma separated list, optional
        // TASK: 090126_07
        sv len = {};
        sv altlen = {};
        sv stride = {};
        sv optional = {}; // can be comma separated, if not present, it is false, value should be provided for every indirection
        sv selector = {}; // only if parameter is union
        sv noautovalidity = {}; // optional
        sv externsync = {}; // optional
        sv objecttype = {}; // optional
        sv validstructs = {}; // optional

        sv type;
        sv name;


        // TODO: the content of the tag should be a valid arbitrary C code after removing xml tags
        std::string stringify;

        static CommandParameter from_xml(const xml::Element& elem, vkg_gen::Arena& arena);

    };


    struct  CommandAlias {
        sv name;
        sv alias;
        sv api = {}; // comma separated list, optional
        sv comment = {}; // optional

        static CommandAlias from_xml(const xml::Element& elem, vkg_gen::Arena& arena);
    };

    class Command {
    public:
        enum class Scope : uint8_t {
            None,
            Inside,
            Outside,
            Both
        };

        // TASK: 090126_06
        sv tasks = {}; // comma separated list, optional
        sv queues = {}; // comma separated list, optional
        sv success_codes = {}; // comma separated list, optional
        sv error_codes = {}; // comma separated list, optional
        Scope render_pass = Scope::None; // optional
        Scope video_encoding = Scope::None; // optional
        sv cmd_buffer_level = {}; // comma separated list, optional
        sv conditional_rendering = {}; // required for vkCmd* commands.Not allowed for other commands. Values true/false
        bool allow_no_queues = false; // optional, default false
        sv export_ = {}; // comma separated list, optional
        sv api = {}; // comma separated list, optional
        sv description = {};

        xml::Element* implicit_extern_sync_params = nullptr;

        sv comment = {};

        sv name;
        sv type; // TODO: link with types? dependencies?

        // TODO: the return type and parameters can contain arbitrary C code
        std::string declatarion;

        std::vector<CommandParameter> parameters = {};

        static Command from_xml(const xml::Element& elem, vkg_gen::Arena& arena);
    };

    struct DefineExt {
        sv name;
        sv value;
        xml::Element& elem;
    };


    template <typename Type>
    concept NameIndexable = requires (Type t) { { t.name } -> std::convertible_to<std::string_view>; };

    class Generator {
        template <NameIndexable Type>
        class NameIndex {
            std::vector<Type*> names;
            std::unordered_map<sv, std::size_t> nameIndex;

        public:
            bool contains(sv name) const { return nameIndex.find(name) != nameIndex.end(); }
            void add(Type* type);
            void add_without_check(Type* type);
            void remove(sv name);
            const std::vector<Type*>& get() const { return names; };
        };

        // Currently expecting only one Platforms tag
        Platforms platforms;

        // Currently expecting only one Tags tag
        Tags tags;

        // CHECK: use unordered map
        // Index of all the types defined in the <types> tags (with the category="enum" included)
        std::map<sv, Type> types = {};
        // Index of all the enums defined in the <enums> tags
        std::map<sv, TypeEnum> enums = {};
        // Index of all the commands defined in the <commands> tags
        std::map<sv, Command> commands = {};
        std::map<sv, CommandAlias> command_aliases = {};

        NameIndex<Type> required_types;
        NameIndex<TypeEnum> required_enums;
        // TODO: think about splitting Alias and Commands
        NameIndex<Command> required_commands;
        NameIndex<CommandAlias> required_commands_aliases;

        std::unordered_set<sv> included_platforms;
        std::unordered_set<sv> included_features;
        std::vector<std::reference_wrapper<xml::Element>> included_extensions;

        // TASK: 100126_01
        std::vector<DefineExt> required_defines;

        void parse_types(vkg_gen::xml::Dom& dom);
        void parse_enums(vkg_gen::xml::Dom& dom);
        void parse_commands(vkg_gen::xml::Dom& dom);

        void generate_enum(TypeEnum& enum_, std::ofstream& file);
        void generate_enum_alias(Type& enum_, std::ofstream& file);
        void generate_struct(Type& struct_, std::ofstream& file);
        void generate_union(Type& struct_, std::ofstream& file);
        void generate_bitmask(Type& bitmask, std::ofstream& file);
        void generate_handle(Type& handle, std::ofstream& file, TypeEnum& obj_enum);

        std::ofstream& generate_command_params(Command& cmd, std::ofstream& file, bool is_end_of_line = true);
        void generate_command(Command& cmd, std::ofstream& file);
        void generate_command_PFN(Command& cmd, std::ofstream& file);
        void generate_command_wrapper(Command& cmd, std::ofstream& file);

        // Wrapper around add_required_type
        void add_required_type(sv name);
        void add_required_type(sv name, std::vector<Type*>& required_types);
        void add_required_enum(sv name);
        void add_required_command(sv name);

        void add_required_version_feature(sv name, vkg_gen::xml::Dom& dom);

        void extend_enum(sv extends, vkg_gen::xml::Element& elem, vkg_gen::Arena& arena, sv block_ext_number = {});

        void add_extension_prototype(sv number, xml::Dom& dom);

    public:
        Generator() {}
        void generate(vkg_gen::xml::Dom& dom, std::ofstream& header, std::ofstream& source, Config& config);

        // TODO:
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

    struct Deprecate {
        sv msg;
    };
    struct LineComment {
        sv comment;
        bool alone = true;
    };

    struct StandaloneComment {
        sv comment;
        bool generate = true;
    };

    inline std::ostream& operator<<(std::ostream& os, Deprecate d) {
        if (!d.msg.empty()) {
            return os << " [[deprecated(\"" << d.msg << "\")]] ";
        }
        return os << ' ';
    };

    inline std::ostream& operator<<(std::ostream& os, LineComment c) {
        if (!c.comment.empty()) {
            if (c.alone)
                os << "// " << c.comment << '\n';
            else
                os << " // " << c.comment;
        }
        return os;
    };

    inline std::ostream& operator<<(std::ostream& os, StandaloneComment c) {
        if (c.generate && !c.comment.empty()) {
            os << "/* " << c.comment << " */";
        }
        return os;
    };

    bool bool_from_string(std::string_view s);
    Command::Scope scope_from_string(std::string_view s);

    template<NameIndexable Type>
    inline void Generator::NameIndex<Type>::add(Type* type) {
        if (contains(type->name))
            return;

        add_without_check(type);
    }

    template<NameIndexable Type>
    inline void Generator::NameIndex<Type>::add_without_check(Type* type) {
        nameIndex[type->name] = names.size();
        names.push_back(type);
    }

    template<NameIndexable Type>
    inline void Generator::NameIndex<Type>::remove(sv name) {
        auto it = nameIndex.find(name);
        if (it != nameIndex.end()) {
            names[it->second] = nullptr;
            nameIndex.erase(it);
        }
    }

} // namespace vkg_gen::Generator



#if 0
void generate_types(vkg_gen::xml::Dom& dom, std::ofstream& file);
void generate_base_types(const vkg_gen::xml::Dom& dom, std::ofstream& file);
void generate_enums(const vkg_gen::xml::Dom& dom, std::ofstream& file);
void generate_API_constants(const vkg_gen::xml::Dom& dom, std::ofstream& file);
#endif


