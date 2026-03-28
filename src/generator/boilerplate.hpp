/**
 * @file boilerplate.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Pregenerated boilerplate code for the generator.
 * @date Created: 23. 03. 2026
 * @date Modified: 28. 03. 2026
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
        "};\n\n"sv,

        "template <typename BitType>\n"sv,
        "constexpr Flags<BitType> operator|(BitType lhs, BitType rhs) noexcept { return Flags<BitType>(lhs) | rhs; }\n"sv,
        "template <typename BitType>\n"sv,
        "constexpr Flags<BitType> operator|(BitType bit, const Flags<BitType>& flags) noexcept { return flags | bit; }\n"sv,
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

    // author: PCJohn (peciva at fit.vut.cz)
    static constexpr std::string_view UNIQUE_HANDLE_DEFINITION =
        "template<typename Type>\n"
        "class UniqueHandle {\n"
        "protected:\n"
        "    Type _value;\n"
        "public:\n"
        "    UniqueHandle() : _value(nullptr) {}\n"
        "    explicit UniqueHandle(Type value) noexcept : _value(value) {}\n"
        "    UniqueHandle(const UniqueHandle&) = delete;\n"
        "    UniqueHandle(UniqueHandle&& other) noexcept : _value(other._value) { other._value = nullptr; }\n"
        "    inline ~UniqueHandle() { destroy(_value); }\n"
        "    UniqueHandle& operator=(const UniqueHandle&) = delete;\n"
        "    UniqueHandle& operator=(UniqueHandle&& other) noexcept { reset(other._value); other._value = nullptr; return *this; }\n"
        "    Type get() const noexcept { return _value; }\n"
        "    void reset(Type value = nullptr) noexcept { if (value == _value) return; destroy(_value); _value = value; }\n"
        "    Type release() noexcept { Type r = _value; _value = nullptr; return r; }\n"
        "    void swap(UniqueHandle& other) { Type tmp = _value; _value = other._value; other._value = tmp; }\n"
        "    explicit operator Type() const noexcept { return _value; }\n"
        "    explicit operator bool() const noexcept { return _value != nullptr; }\n"
        "};\n\n"sv;

    // author: PCJohn (peciva at fit.vut.cz)
    static constexpr std::string_view DETAIL_NAMESPACE =
        "namespace detail {\n"
        "    extern void* _library;\n"
        "    extern Instance _instance;\n"
        "    extern PhysicalDevice _physicalDevice;\n"
        "    extern Device _device;\n"
        "    extern uint32_t _instanceVersion;\n"
        "}\n\n"
        "inline void* library() { return detail::_library; }\n"
        "inline Instance instance() { return detail::_instance; }\n"
        "inline PhysicalDevice physicalDevice() { return detail::_physicalDevice; }\n"
        "inline Device device() { return detail::_device; }\n"
        "inline uint32_t enumerateInstanceVersion() { return detail::_instanceVersion; }\n\n"sv;

    // author: PCJohn (peciva at fit.vut.cz)
    static constexpr std::string_view INIT_DECLARATIONS =
        "void loadLib_throw();\n"
        "Result loadLib_noThrow() noexcept;\n"
        "inline void loadLib() { loadLib_throw(); }\n\n" // TODO: respect config with the throw_nothrow behavior
        "void loadLib_throw(const char* libPath);\n"
        "Result loadLib_noThrow(const char* libPath) noexcept;\n"
        "inline void loadLib(const char* libPath) { loadLib_throw(libPath); }\n\n"
        "void initInstance_throw(const InstanceCreateInfo& createInfo);\n"
        "Result initInstance_noThrow(const InstanceCreateInfo& createInfo) noexcept;\n"
        "inline void initInstance(const InstanceCreateInfo& createInfo) { initInstance_throw(createInfo); }\n"
        "void initInstance(Instance instance) noexcept;\n\n"
        "void initDevice_throw(PhysicalDevice pd, const DeviceCreateInfo& createInfo);\n"
        "Result initDevice_noThrow(PhysicalDevice pd, const DeviceCreateInfo& createInfo) noexcept;\n"
        "inline void initDevice(PhysicalDevice pd, const DeviceCreateInfo& createInfo) { initDevice_throw(pd, createInfo); }\n"
        "void initDevice(PhysicalDevice physicalDevice, Device device) noexcept;\n\n"
        "template<typename Qual> inline Qual getInstanceProcAddr(const char* name) noexcept;\n"
        "template<typename Qual> inline Qual getDeviceProcAddr(const char* name) noexcept;\n"
        "void destroyDevice() noexcept;\n"
        "void destroyInstance() noexcept;\n"
        "void unloadLib() noexcept;\n"
        "void cleanUp() noexcept;\n\n"sv;

    // author: PCJohn (peciva at fit.vut.cz)
    static constexpr std::string_view VERSION_HELPERS =
        "constexpr uint32_t makeApiVersion(uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch) { return (variant << 29) | (major << 22) | (minor << 12) | patch; }\n"
        "constexpr uint32_t makeApiVersion(uint32_t major, uint32_t minor, uint32_t patch) { return (major << 22) | (minor << 12) | patch; }\n"
        "constexpr const uint32_t ApiVersion10 = makeApiVersion(0, 1, 0, 0);\n"
        "constexpr const uint32_t ApiVersion11 = makeApiVersion(0, 1, 1, 0);\n"
        "constexpr const uint32_t ApiVersion12 = makeApiVersion(0, 1, 2, 0);\n"
        "constexpr const uint32_t ApiVersion13 = makeApiVersion(0, 1, 3, 0);\n"
        "constexpr const uint32_t ApiVersion14 = makeApiVersion(0, 1, 4, 0);\n"
        "constexpr uint32_t apiVersionVariant(uint32_t version) { return version >> 29; }\n"
        "constexpr uint32_t apiVersionMajor(uint32_t version) { return (version >> 22) & 0x7f; }\n"
        "constexpr uint32_t apiVersionMinor(uint32_t version) { return (version >> 12) & 0x3ff; }\n"
        "constexpr uint32_t apiVersionPatch(uint32_t version) { return version & 0xfff; }\n\n"sv;

    // author: PCJohn (peciva at fit.vut.cz)
    static constexpr std::string_view ERROR_BASE_CLASS =
        "void throwResultExceptionWithMessage(Result result, const char* message);\n"
        "void throwResultException(Result result, const char* funcName);\n"
        "inline void checkForSuccessValue(Result result, const char* funcName) { if (result != Result::eSuccess) throwResultException(result, funcName); }\n"
        "inline void checkSuccess(Result result, const char* funcName) { if (int32_t(result) < 0) throwResultException(result, funcName); }\n\n"
        "class Error {\n"
        "protected:\n"
        "    char* _msg = nullptr;\n"
        "public:\n"
        "    Error() noexcept = default;\n"
        "    Error(const char* message) noexcept { if (message == nullptr) { _msg = nullptr; return; } size_t n = strlen(message) + 1; _msg = reinterpret_cast<char*>(malloc(n)); if (_msg) strncpy(_msg, message, n); }\n"
        "    Error(const char* msgHeader, const char* msgBody) noexcept;\n"
        "    Error(const char* funcName, Result result) noexcept;\n"
        "    Error(const Error& other) noexcept { delete[] _msg; if (other._msg == nullptr) { _msg = nullptr; return; } size_t n = strlen(other._msg) + 1; _msg = reinterpret_cast<char*>(malloc(n)); if (_msg) strncpy(_msg, other._msg, n); }\n"
        "    Error& operator=(const Error& rhs) noexcept { if (rhs._msg == nullptr) { _msg = nullptr; return *this; } size_t n = strlen(rhs._msg) + 1; _msg = reinterpret_cast<char*>(malloc(n)); if (_msg) strncpy(_msg, rhs._msg, n); return *this; }\n"
        "    virtual ~Error() noexcept { delete[] _msg; }\n"
        "    virtual const char* what() const noexcept { return _msg ? _msg : \"Unknown exception\"; }\n"
        "};\n\n"
        "class VkgError : public Error { public: using Error::Error; };\n\n"sv;

    static constexpr std::string_view PROC_ADDR_HELPERS =
        "template<typename Qual> inline Qual getGlobalProcAddr(const char* name) noexcept { return reinterpret_cast<Qual>(funcs.vkGetInstanceProcAddr(nullptr, name)); }\n"
        "template<typename Qual> inline Qual getInstanceProcAddr(const char* name) noexcept { return reinterpret_cast<Qual>(funcs.vkGetInstanceProcAddr(detail::_instance.handle(), name)); }\n"
        "template<typename Qual> inline Qual getDeviceProcAddr(const char* name) noexcept { return reinterpret_cast<Qual>(funcs.vkGetDeviceProcAddr(detail::_device.handle(), name)); }\n\n"sv;

    static constexpr std::string_view CPP_IMPL =
        "#include <dlfcn.h>\n"
        "#include <assert.h>\n"
        "\n"
        "void* detail::_library = nullptr;\n"
        "Instance detail::_instance = nullptr;\n"
        "PhysicalDevice detail::_physicalDevice = nullptr;\n"
        "Device detail::_device = nullptr;\n"
        "uint32_t detail::_instanceVersion = 0;\n"
        "Funcs detail::_funcs;\n\n"sv;

    static constexpr std::string_view LOAD_UNLOAD_LIB_IMPL =
        "void loadLib_throw(const char* libPath) {\n"
        "    detail::_library = dlopen(libPath, RTLD_NOW);\n"
        "    if (!detail::_library) throw VkgError(\"Failed to load Vulkan library\");\n"
        "    funcs.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)(dlsym(detail::_library, \"vkGetInstanceProcAddr\"));\n"
        "    if (!funcs.vkGetInstanceProcAddr) throw VkgError(\"Failed to load vkGetInstanceProcAddr\");\n"
        "    funcs.vkCreateInstance = getGlobalProcAddr<PFN_vkCreateInstance>(\"vkCreateInstance\");\n"
        "    funcs.vkEnumerateInstanceExtensionProperties = getGlobalProcAddr<PFN_vkEnumerateInstanceExtensionProperties>(\"vkEnumerateInstanceExtensionProperties\");\n"
        "    funcs.vkEnumerateInstanceLayerProperties = getGlobalProcAddr<PFN_vkEnumerateInstanceLayerProperties>(\"vkEnumerateInstanceLayerProperties\");\n"
        "}\n\n"
        "void loadLib_throw() { loadLib_throw(\"libvulkan.so.1\"); }\n\n"
        "Result loadLib_noThrow(const char* libPath) noexcept {\n"
        "    detail::_library = dlopen(libPath, RTLD_NOW);\n"
        "    if (!detail::_library) return Result::eErrorInitializationFailed;\n"
        "    funcs.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)(dlsym(detail::_library, \"vkGetInstanceProcAddr\"));\n"
        "    if (!funcs.vkGetInstanceProcAddr) return Result::eErrorInitializationFailed;\n"
        "    funcs.vkCreateInstance = getGlobalProcAddr<PFN_vkCreateInstance>(\"vkCreateInstance\");\n"
        "    funcs.vkEnumerateInstanceExtensionProperties = getGlobalProcAddr<PFN_vkEnumerateInstanceExtensionProperties>(\"vkEnumerateInstanceExtensionProperties\");\n"
        "    funcs.vkEnumerateInstanceLayerProperties = getGlobalProcAddr<PFN_vkEnumerateInstanceLayerProperties>(\"vkEnumerateInstanceLayerProperties\");\n"
        "    return Result::eSuccess;\n"
        "}\n\n"
        "Result loadLib_noThrow() noexcept { return loadLib_noThrow(\"libvulkan.so.1\"); }\n\n"
        "void unloadLib() noexcept {\n"
        "    if (detail::_library) {\n"
        "        dlclose(detail::_library);\n"
        "        detail::_library = nullptr;\n"
        "    }\n"
        "}\n\n"
        "void cleanUp() noexcept {\n"
        "    destroyDevice();\n"
        "    destroyInstance();\n"
        "    unloadLib();\n"
        "}\n\n"
        "void destroyDevice() noexcept {\n"
        "    if (detail::_device) {\n"
        "        funcs.vkDestroyDevice(detail::_device.handle(), nullptr);\n"
        "        detail::_device = nullptr;\n"
        "    }\n"
        "}\n\n"
        "void destroyInstance() noexcept {\n"
        "    if (detail::_instance) {\n"
        "        funcs.vkDestroyInstance(detail::_instance.handle(), nullptr);\n"
        "        detail::_instance = nullptr;\n"
        "    }\n"
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
