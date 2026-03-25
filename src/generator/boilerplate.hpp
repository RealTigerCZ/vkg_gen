/**
 * @file boilerplate.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Pregenerated boilerplate code for the generator.
 * @date Created: 23. 03. 2026
 * @date Modified: 25. 03. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once
#include <ostream>
#include <array>
#include <string_view>

namespace boilerplate {
    using namespace std::string_view_literals;

    // author: PCJohn (peciva at fit.vut.cz)
    static constexpr std::array HANDLE_DEFINITION = {
        "// FIXME: \n"sv,
        "template<typename Type> class UniqueHandle;\n\n"sv,

        "template<typename T>\n"sv,
        "class Handle {\n"sv,
        "    protected:\n"sv,
        "        T _handle;\n"sv,
        "    public:\n"sv,
        "        using HandleType = T;\n\n"sv,

        "        Handle() noexcept {}\n"sv,
        "        Handle(std::nullptr_t) noexcept : _handle(nullptr) {}\n"sv,
        "        Handle(T nativeHandle) noexcept : _handle(nativeHandle) {}\n"sv,
        "        Handle(const Handle& h) noexcept : _handle(h._handle) {}\n"sv,
        "        Handle(const UniqueHandle<Handle<T>>& u) noexcept : _handle(u.get().handle()) {}\n"sv,
        "        Handle& operator=(const Handle rhs) noexcept { _handle = rhs._handle; return *this; }\n"sv,
        "        Handle& operator=(const UniqueHandle<T>& rhs) noexcept { _handle = rhs._handle; return *this; }\n"sv,
        "        T handle() const noexcept { return _handle; }\n"sv,
        "        explicit operator bool() const noexcept { return _handle != nullptr; }\n"sv,
        "        bool operator==(const Handle rhs) const noexcept { return _handle == rhs._handle; }\n"sv,
        "        bool operator!=(const Handle rhs) const noexcept { return _handle != rhs._handle; }\n"sv,
        "};\n"sv,
    };

    // original author: PCJohn (peciva at fit.vut.cz) modified by Jaroslav Hucel
    static constexpr std::array FLAGS_DEFINITION = {
        "template <typename BitType>\n"sv,
        "class Flags {\n"sv,
        "    public:\n\n"sv,

        "    using ValueType = __underlying_type(BitType);\n"sv,

        "   protected:\n"sv,
        "       ValueType _value;\n"sv,
        "   public:\n\n"sv,

        "   // constructors\n"sv,
        "   constexpr Flags() noexcept : _value(0) {}\n"sv,
        "   constexpr Flags(BitType bit) noexcept : _value(static_cast<ValueType>(bit)) {}\n"sv,
        "   constexpr Flags(const Flags<BitType>& other) noexcept = default;\n"sv,
        "   constexpr explicit Flags(ValueType flags) noexcept : _value(flags) {}\n\n"sv,

        "   // relational operators\n"sv,
        "   constexpr bool operator< (const Flags<BitType>& rhs) const noexcept { return _value < rhs._value; }\n"sv,
        "   constexpr bool operator<=(const Flags<BitType>& rhs) const noexcept { return _value <= rhs._value; }\n"sv,
        "   constexpr bool operator> (const Flags<BitType>& rhs) const noexcept { return _value > rhs._value; }\n"sv,
        "   constexpr bool operator>=(const Flags<BitType>& rhs) const noexcept { return _value >= rhs._value; }\n"sv,
        "   constexpr bool operator==(const Flags<BitType>& rhs) const noexcept { return _value == rhs._value; }\n"sv,
        "   constexpr bool operator!=(const Flags<BitType>& rhs) const noexcept { return _value != rhs._value; }\n\n"sv,

        "   // logical operator\n"sv,
        "   constexpr bool operator!() const noexcept { return !_value; }\n\n"sv,

        "   // bitwise operators\n"sv,
        "   constexpr Flags<BitType> operator&(const Flags<BitType>& rhs) const noexcept { return Flags<BitType>(_value & rhs._value); }\n"sv,
        "   constexpr Flags<BitType> operator|(const Flags<BitType>& rhs) const noexcept { return Flags<BitType>(_value | rhs._value); }\n"sv,
        "   constexpr Flags<BitType> operator^(const Flags<BitType>& rhs) const noexcept { return Flags<BitType>(_value ^ rhs._value); }\n\n"sv,

        "   // assignment operators\n"sv,
        "   constexpr Flags<BitType>& operator= (const Flags<BitType>& rhs) noexcept = default;\n"sv,
        "   constexpr Flags<BitType>& operator|=(const Flags<BitType>& rhs) noexcept { _value |= rhs._value; return *this; }\n"sv,
        "   constexpr Flags<BitType>& operator&=(const Flags<BitType>& rhs) noexcept { _value &= rhs._value; return *this; }\n"sv,
        "   constexpr Flags<BitType>& operator^=(const Flags<BitType>& rhs) noexcept { _value ^= rhs._value; return *this; }\n\n"sv,

        "   // cast operators\n"sv,
        "   explicit constexpr operator bool() const noexcept { return !!_value; }\n"sv,
        "   explicit constexpr operator ValueType() const noexcept { return _value; }\n"sv,
        "};\n\n"sv,

        "// These specializations are used for dummy flags, that don't have any bits set defined\n"sv,
        "template <>\n"sv,
        "class Flags<uint32_t> {\n"sv,
        "    uint32_t _value;\n"sv,
        "public:\n"sv,
        "    constexpr Flags(uint32_t v = 0) noexcept : _value(v) {}\n"sv,
        "    constexpr operator uint32_t() const noexcept { return _value; }\n"sv,
        "};\n\n"sv,

        "template <>\n"sv,
        "class Flags<uint64_t> {\n"sv,
        "    uint64_t _value;\n"sv,
        "public:\n"sv,
        "    constexpr Flags(uint64_t v = 0) noexcept : _value(v) {}\n"sv,
        "    constexpr operator uint64_t() const noexcept { return _value; }\n"sv,
        "};\n"sv,
    };


    // FIXME:
    static constexpr std::string_view VIDEO_INCLUDES =
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
        "#include \"vk_video/vulkan_video_codec_vp9std.h\"\n"sv;

    // FIXME:
    static constexpr std::string_view HEADER_EXTERNS =
        "extern void load_lib(const char* path);\n"
        "extern void unload_lib();\n"
        "extern void init_vk_functions();\n"sv;

    static constexpr std::string_view CPP_IMPL =
        "#include <dlfcn.h>\n"
        "#include <assert.h>\n"
        "\n"
        "void* lib = nullptr;\n"
        "Funcs funcs;\n"
        "static ExtensionProperties * extensions = nullptr;\n"
        "static uint32_t extensions_count = 0;\n"sv;

    static constexpr std::string_view LOAD_UNLOAD_LIB_IMPL =
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
        "    extensions = new ExtensionProperties[extensions_count];\n"
        "    char** extensions_names = new char* [extensions_count];\n"
        "    funcs.vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, extensions);\n"
        "\n"
        "    for (uint32_t i = 0; i < extensions_count; i++) {\n"
        "        extensions_names[i] = extensions[i].extensionName;\n"
        "    }\n"
        "\n"
        "    ApplicationInfo app_info = {\n"
        "                .pApplicationName = \"test\",\n"
        "                .applicationVersion = 0,\n"
        "                .pEngineName = nullptr,\n"
        "                .engineVersion = 0,\n"
        "                .apiVersion = 1 << 22 };\n"
        "\n"
        "    InstanceCreateInfo info = {\n"
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
        "}\n"sv;

    inline std::ostream& print(std::ostream& os, std::string_view text, int indent = 0) {
        return os << std::string(indent * 4, ' ') << text;
    }

    template<size_t N>
    inline std::ostream& print(std::ostream& os, const std::array<std::string_view, N>& lines, int indent = 0) {
        std::string pad(indent * 4, ' ');
        for (auto& line : lines)
            os << pad << line;
        return os;
    }
}
