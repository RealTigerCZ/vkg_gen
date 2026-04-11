/**
 * @file generator.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief
 * @date Created: 02. 11. 2025
 * @date Modified: 11. 04. 2026
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

        static Platform from_xml(const xml::Element& elem);
    };

    struct Platforms {
        sv comment = {};
        std::vector<Platform> platforms = {};

        static Platforms from_xml(const xml::Element& elem);
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

    class ApiType {
    public:
        enum class Value : uint8_t {
            Any = 0xff,
            Vulkan = 1 << 0,
            VulkanSC = 1 << 1,
        };
        using enum Value;
        using underlying_type = std::underlying_type_t<Value>;

        constexpr ApiType() : value(Any) {}
        constexpr ApiType(Value val) : value(val) {}
        constexpr operator Value() const { return value; }
        explicit operator bool() const { return !!(underlying_type)value; }
        constexpr bool operator==(ApiType a) const { return value == a.value; }
        constexpr bool operator!=(ApiType a) const { return value != a.value; }
        constexpr ApiType operator&(ApiType b) const { return ApiType((Value)((underlying_type)value & (underlying_type)b.value)); }
        constexpr ApiType operator|(ApiType b) const { return ApiType((Value)((underlying_type)value | (underlying_type)b.value)); }
        constexpr ApiType operator&(Value b) const { return ApiType((Value)((underlying_type)value & (underlying_type)b)); }
        constexpr ApiType operator|(Value b) const { return ApiType((Value)((underlying_type)value | (underlying_type)b)); }

        bool is_vulkan() const { return (underlying_type)value & (underlying_type)Vulkan; }

        static ApiType from_string(sv str);
    private:
        Value value;
    };

    struct HandleInfo;
    struct TypeHandle {
        // Name of the enum class containing VK_OBJECT_TYPE_*
        static constexpr sv obj_enum_name = "VkObjectType";

        sv parent; // Notes another type with the 'handle'
        // category that acts as a parent object for
        // this type.

        sv objtypeenum; // name of VK_OBJECT_TYPE_* API enumerant which
        // corresponds to this type. Should be present

        HandleInfo* info = nullptr; // FIXME: TASK: 310326_01 Currently we create this info in the cache_handles function into array, that then is cleared, THIS IS A BIG HACK and needs refactoring
    };

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
            ApiType api;
            bool is_alias = false;
            bool is_standalone_comment = false; //TODO:
            uint16_t ext_number = 0;

            static EnumItem from_xml(const vkg_gen::xml::Element& elem, vkg_gen::Arena& arena, TypeEnum* parent, bool is_standalone_comment = false, bool extend_parent = false, uint16_t ext_number = 0);
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
        sv protect; // platform protection macro (e.g., VK_USE_PLATFORM_WIN32_KHR)
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


    class TypeParam {
    public:
        enum class PreQualifier : uint8_t {
            None = 0,
            Const = 1 << 0,
            Struct = 1 << 1,
            Union = 1 << 2,
            Enum = 1 << 3,
            ConstStruct = Const | Struct,
            ConstUnion = Const | Union,
            ConstEnum = Const | Enum
        };

        enum class PostQualifier : uint8_t {
            None = 0,
            Const = 1 << 0,
            Pointer = 1 << 1,
            ConstPointer = Const | Pointer
        };


        struct ArrayExtension {
            enum class Type : uint8_t {
                PlainSize,
                EnumItem,
                BitSelect,
            };
            Type type;
            union {
                TypeEnum::EnumItem* enum_item;
                uint64_t size;
                uint8_t bitpos;
            };
            ArrayExtension(uint64_t s) : type(Type::PlainSize) { size = s; }
            ArrayExtension(TypeEnum::EnumItem* e) : type(Type::EnumItem) { enum_item = e; }
            ArrayExtension(uint8_t b) : type(Type::BitSelect) { bitpos = b; }
        };

        sv type;
        sv name;
        PreQualifier pre_qual = PreQualifier::None;
        sv comment = {}; // TODO: this comment is in the moment not generated
        std::vector<PostQualifier> post_quals;
        std::vector<ArrayExtension> array_extensions;

        enum class State : uint8_t {
            AtStart,
            BeforeType,
            BeforeName,
            AfterName,
            InsideArray,
            AfterBitSelect
        };

        static TypeParam from_xml(const xml::Element& elem, vkg_gen::Arena& arena);
        // TODO: replace this with operator<<
        std::string stringify() const;
        bool is_const() const;

    private:
        static uint64_t get_size(sv str);
        State parse_string(sv text, State state);
    };

    void trim_leading_ws(sv& s);
    void trim_trailing_ws(sv& s);
    void trim_ws(sv& s);


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

        TypeParam type_param;

        ApiType api;

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

        sv comment;
        bool is_standalone_comment = false;

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

    struct TypeRef {
        bool is_const = false;
        sv type;
        std::vector<TypeParam::PostQualifier> post_quals;

        static TypeRef from_string(sv text);
        std::string stringify() const;
    };

    struct TypeFuncpointer {
        TypeRef return_type;
        std::vector<TypeParam> parameters;
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
        ApiType api; // api attribute, matches a <feature> api attribute, if present
        sv alias; // alias attribute
        union {
            sv requires_ = {}; // requires attribute, pointing to another type
            sv bitvalues; // Bitmask only: FlagBits enum name. Populated from XML `requires` (old-style) or `bitvalues` (new-style) attr.
        };
        sv comment; // comment attribute
        sv deprecated; // reason for deprecation if present, can also have values "true" or "false"
        sv protect; // platform protection macro, set internally (not from XML)
        Category category = Category::None;
        State state = State::NotUsed;
        union {
            TypeHandle* handle;
            TypeStruct* struct_;
            TypeUnion* union_;
            TypeFuncpointer* funcptr;
        };

        const xml::Element& elem;

        // TODO: change to static from_xml(...)
        Type(const xml::Element& elem, vkg_gen::Arena& arena);
        Type(Type&& other) noexcept;
        Type& operator=(Type&&) = delete;
        Type(const Type&) = delete;
        Type& operator=(const Type&) = delete;
        ~Type();

    private:
        Category category_from_string(sv s) const;
        void parse_struct(const xml::Element& elem, Arena& arena);
        void parse_union(const xml::Element& elem, Arena& arena);
        void parse_funcpointer(const xml::Element& elem, Arena& arena);
    };

    class CommandParameter {
    public:
        ApiType api = ApiType::Any;
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

        TypeParam type_param;

        static CommandParameter from_xml(const xml::Element& elem, vkg_gen::Arena& arena);

    };


    struct  CommandAlias {
        sv name;
        sv alias;
        ApiType api = ApiType::Any; // comma separated list, optional
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
        ApiType api = {}; // comma separated list, optional
        sv description = {};

        xml::Element* implicit_extern_sync_params = nullptr;

        sv comment = {};
        sv protect; // platform protection macro, set internally (not from XML)

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

    enum class Extension {
        None,
        Img,
        Amd,
        Amdx,
        Arm,
        Fsl,
        Brcm,
        Nxp,
        Nv,
        Nvx,
        Viv,
        Vsi,
        Kdab,
        Android,
        Chromium,
        Fuchsia,
        Ggp,
        Google,
        Qcom,
        Lunarg,
        Nzxt,
        Samsung,
        Sec,
        Tizen,
        Renderdoc,
        Nn,
        Mvk,
        Khr,
        Khx,
        Ext,
        Mesa,
        Intel,
        Huawei,
        Ohos,
        Valve,
        Qnx,
        Juice,
        Fb,
        Rastergrid,
        Msft,
        Shady,
        Fredemmott,
        Mtk,
    };

    struct CommandClassification {
        enum class Pattern : uint8_t {
            Void,           // void return, no error checking
            ResultVoid,     // VkResult return, no output data
            ResultCreate,   // VkResult return, creates handle via out-param
            VoidOutParam,   // void return, fills struct via out-param
            Enumerate,      // two-call enumerate pattern
            ResultOutParam, // VkResult + non-handle out-param
            Other,          // non-standard return type — pass-through
        };
        Pattern pattern;
        int implicit_param_idx = -1;     // device/instance to substitute with global
        sv implicit_global;              // implicit parameter replacement: "detail::_device" or "detail::_instance"
        int output_param_idx = -1;       // handle or struct output
        int allocator_param_idx = -1;    // VkAllocationCallbacks*
        int count_param_idx = -1;        // for enumerate
        int array_param_idx = -1;        // for enumerate
        bool output_has_destroy = false; // ResultCreate: output handle has destroy() overload

        // Input array pairs: non-pointer uint32_t count + const T* with len referencing it (1:1 only)
        static constexpr int max_input_arrays = 8;
        struct InputArrayPair { int count_idx; int array_idx; };
        InputArrayPair input_arrays[max_input_arrays] = {};
        int input_array_count = 0;

        bool is_input_count(int idx) const {
            for (int k = 0; k < input_array_count; ++k)
                if (input_arrays[k].count_idx == idx) return true;
            return false;
        }
        bool is_input_array(int idx) const {
            for (int k = 0; k < input_array_count; ++k)
                if (input_arrays[k].array_idx == idx) return true;
            return false;
        }
    };

    struct HandleInfo {
        enum class Kind : uint8_t {
            InstanceItself, // VkInstance
            DeviceItself,   // VkDevice
            Instance,       // Owned by Instance (e.g. VkSurfaceKHR)
            Device,         // Owned by Device (e.g. VkBuffer)
        };

        enum class DestroyBehavior : uint8_t {
            None,       // No cleanup command (e.g. PhysicalDevice, Queue)
            Destroy,    // vkDestroy* — takes (parent, handle, alloc)
            Free,       // vkFree* — simple free like vkFreeMemory(device, mem, alloc)
            Release,    // vkRelease* — vkReleasePerformanceConfigurationINTEL
        };

        const Type& type;
        Kind kind;
        DestroyBehavior destroy_behavior = DestroyBehavior::None;
        const Command* destroy_command = nullptr; // the vkDestroy*/vkFree* command, if any

        bool is_destroyable() const { return destroy_command != nullptr; }
    };

    struct NameTranslator {
        static inline bool keep_av1_vp9 = true;

        struct TransformedEnumName {
            const std::string name;
            const Extension ext = Extension::None;
        };

        std::string new_name;

        NameTranslator(std::string&& new_name) : new_name(std::move(new_name)) {}

        static TransformedEnumName transform_enum_name(sv name, bool is_bitmask);
        static NameTranslator from_enum_value(sv value, const TransformedEnumName& enum_class_transformed, bool is_bitmask);
        static NameTranslator from_type_name(sv name); // enums, structs, unions
        static NameTranslator from_constexpr_value(sv value_name);
        static NameTranslator from_command_name(sv name); // vkCreateBuffer -> createBuffer
        static NameTranslator from_input_array_name(sv name); // pViewports -> viewports
        static std::pair<std::string, std::string> unique_command_name(sv name); // vkCreateBuffer -> createBufferUnique, createBuffer

    protected:
        static void transform_from_upper_constant(std::string& name, size_t start_pos, bool first_is_upper);
        static Extension get_and_remove_extension_constant(sv& name);
        static Extension get_and_remove_extension_name(sv& name);
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

        std::unordered_map<sv, sv> platform_to_protect; // platform name → protect macro
        std::unordered_set<sv> included_platforms;

        std::unordered_map<sv, xml::Element*> extension_name_to_element; // TASK: 110426_01
        std::unordered_set<xml::Element*> processed_extensions;

        Config config;

        // TASK: 100126_01
        std::vector<DefineExt> required_defines;

        void parse_platforms(vkg_gen::xml::Dom& dom);
        void parse_types(vkg_gen::xml::Dom& dom);
        void parse_enums(vkg_gen::xml::Dom& dom);
        void parse_commands(vkg_gen::xml::Dom& dom);
        bool is_handle(sv type);
        void cache_handles(std::vector<HandleInfo>& handles);


        void generate_enum(TypeEnum& enum_, std::ofstream& file);
        void generate_enum_alias(const Type& enum_, std::ofstream& file);
        void generate_member(Member& member, std::ofstream& file, sv struct_union, sv parent_name);
        void generate_struct_union(const Type& type, std::ofstream& file, sv struct_union);
        void generate_struct(const Type& struct_, std::ofstream& file);
        void generate_union(const Type& union_, std::ofstream& file);
        void generate_bitmask(const Type& bitmask, std::ofstream& file);
        void generate_handle(const Type& handle, std::ofstream& file, TypeEnum& obj_enum);
        void generate_error_classes(std::ofstream& file);


        std::ofstream& generate_command_params(Command& cmd, std::ofstream& file, bool is_end_of_line = true);
        void generate_command(Command& cmd, std::ofstream& file);
        void generate_command_PFN(Command& cmd, std::ofstream& file);
        void generate_command_wrapper(Command& cmd, std::ofstream& file);
        void generate_funcpointer(Type& type, std::ofstream& file);

        CommandClassification classify_command(const Command& cmd);
        void detect_input_arrays(const Command& cmd, CommandClassification& cc);
        bool has_pnext(sv struct_type_name);

        bool should_emit_throw() const;
        bool should_emit_nothrow() const;
        bool should_emit_default() const;
        bool default_is_throw() const;

        // Emits a single parameter in C++ wrapper style (const struct ptr → const ref, VkHandle → Handle, else raw stringify)
        void emit_wrapper_param(const CommandParameter& param, std::ofstream& file);
        // Emits comma-separated param names for forwarding calls. skip_idx: extra index to skip (PD param)
        void emit_forward_param_names(const Command& cmd, const CommandClassification& cc,
            std::ofstream& file, bool include_nothrow_output = false, int skip_idx = -1);
        // Emits "(params, vector<ElemType>& v)" for enumerate _noThrow signatures. skip_idx for PD overloads.
        void emit_enumerate_nothrow_params(const Command& cmd, const CommandClassification& cc,
            std::ofstream& file, int skip_idx = -1);
        // Generates entire parameter list
        void generate_wrapper_params(const Command& cmd, const CommandClassification& cc,
            std::ofstream& file, bool for_nothrow_output = false, int skip_idx = -1);
        // Generates the argument list for a wrapper function call
        void generate_call_args(const Command& cmd, const CommandClassification& cc,
            std::ofstream& file, bool for_nothrow = false);

        void generate_wrapper_void(const Command& cmd, const CommandClassification& cc, std::ofstream& file);
        void generate_wrapper_result_void(const Command& cmd, const CommandClassification& cc, std::ofstream& file);
        void generate_wrapper_result_create(const Command& cmd, const CommandClassification& cc, std::ofstream& file);
        void generate_wrapper_void_outparam(const Command& cmd, const CommandClassification& cc, std::ofstream& file);
        void generate_wrapper_result_outparam(const Command& cmd, const CommandClassification& cc, std::ofstream& file);

        void generate_wrapper_enumerate_header(const Command& cmd, const CommandClassification& cc, std::ofstream& file);
        void generate_wrapper_enumerate_source(const Command& cmd, const CommandClassification& cc, std::ofstream& file);
        void generate_enumerate_call(const Command& cmd, const CommandClassification& cc, std::ofstream& file, bool second_call);
        void generate_throw_result_exception(std::ofstream& source);
        void generate_physical_device_overloads(const Command& cmd, const CommandClassification& cc, std::ofstream& file);
        void generate_physical_device_overload_alias(const CommandAlias& cmd, std::ofstream& file);
        void generate_unique_handle_create(const Command& cmd, const CommandClassification& cc, std::ofstream& file);


        bool is_device_level_command(const Command& cmd) const;
        std::pair<bool, bool> is_handle_or_const_ptr_struct_argument(const CommandParameter& param);

        NameTranslator get_translated_type_name(sv name);

        // Wrapper around add_required_type
        void add_required_type(sv name);
        void add_required_type(sv name, std::vector<Type*>& required_types);
        void add_required_enum(sv name);
        void add_required_command(sv name);

        void add_required_version_feature(sv name, vkg_gen::xml::Dom& dom);

        void extend_enum(sv extends, vkg_gen::xml::Element& elem, vkg_gen::Arena& arena, uint16_t block_ext_number = 0, sv protect = {});

        void add_extension_prototype(xml::Element& ext, xml::Dom& dom);
        void add_extension_with_deps(xml::Element& ext, xml::Dom& dom, sv required_by = {});

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
        static inline bool enabled;
        sv msg;
        bool generate = true;
    };
    struct LineComment {
        sv comment;
        bool alone = true;
        bool generate = true;
    };

    struct StandaloneComment {
        sv comment;
        bool generate = true;
    };

    template <typename Qual>
    concept IsTypeQualifier = std::same_as<Qual, TypeParam::PreQualifier> ||
        std::same_as<Qual, TypeParam::PostQualifier>;

    template <IsTypeQualifier Qual>
    constexpr Qual operator|(Qual a, Qual b) {
        return static_cast<Qual>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    template <IsTypeQualifier Qual>
    constexpr Qual operator&(Qual a, Qual b) {
        return static_cast<Qual>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    template <IsTypeQualifier Qual>
    constexpr Qual& operator|=(Qual& a, Qual b) {
        return a = a | b;
    }

    inline bool TypeParam::is_const() const { return bool(pre_qual & PreQualifier::Const); };

    inline std::ostream& operator<<(std::ostream& os, const NameTranslator& translator) {
        return os << translator.new_name;
    }

    inline std::ostream& operator<<(std::ostream& os, Deprecate d) {
        if (d.generate && d.enabled && !d.msg.empty()) {
            return os << " [[deprecated(\"" << d.msg << "\")]] ";
        }
        return os << ' ';
    };

    inline std::ostream& operator<<(std::ostream& os, LineComment c) {
        if (c.generate && !c.comment.empty()) {
            if (c.alone)
                os << "// " << c.comment << '\n';
            else
                os << " // " << c.comment;
        }
        return os;
    };

    inline std::ostream& operator<<(std::ostream& os, StandaloneComment c) {
        if (c.generate && !c.comment.empty()) {
            os << "/* " << c.comment << " */\n";
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

    inline bool Generator::should_emit_throw() const {
        return config.exception_behavior != ExceptionBehavior::NoThrowOnly;
    }
    inline bool Generator::should_emit_nothrow() const {
        return config.exception_behavior != ExceptionBehavior::ThrowOnly;
    }
    inline bool Generator::should_emit_default() const {
        return config.exception_behavior == ExceptionBehavior::BothWithDefaultThrow
            || config.exception_behavior == ExceptionBehavior::BothWithDefaultNoThrow;
    }
    inline bool Generator::default_is_throw() const {
        return config.exception_behavior == ExceptionBehavior::BothWithDefaultThrow;
    }

    inline std::pair<bool, bool> Generator::is_handle_or_const_ptr_struct_argument(const CommandParameter& param) {
        Type::Category cat = types.at(param.type_param.type).category;
        bool param_is_handle = config.generate_handle_class && cat == Type::Category::Handle;
        bool is_const_struct_ptr = param.type_param.is_const()
            && param.type_param.post_quals.size() == 1
            && bool(param.type_param.post_quals[0] & TypeParam::PostQualifier::Pointer) // TODO: can't use PostQualifier::ConstPointer because the const qualifier is before type name (in pre-qualifiers)
            && !param_is_handle
            && (cat == Type::Category::Struct || cat == Type::Category::Union);
        return { param_is_handle, is_const_struct_ptr };
    }

} // namespace vkg_gen::Generator



#if 0
void generate_types(vkg_gen::xml::Dom& dom, std::ofstream& file);
void generate_base_types(const vkg_gen::xml::Dom& dom, std::ofstream& file);
void generate_enums(const vkg_gen::xml::Dom& dom, std::ofstream& file);
void generate_API_constants(const vkg_gen::xml::Dom& dom, std::ofstream& file);
#endif


