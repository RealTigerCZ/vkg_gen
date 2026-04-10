#pragma once

// THIS FILE MAY BE DELETED IN FUTURE
// It is here only for debugging purposes

#include <cassert>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>

#if defined(__GNUC__) && !defined(__clang__) && __has_include(<stacktrace>)
#include <stacktrace>
#include <ranges>
#include <span>

class tracable_runtime_error : public std::runtime_error {
    static std::string transform_message(std::string_view msg) {
        std::stringstream ss;
        ss << "ERROR: " << msg << "\nStacktrace:\n";
        auto trace = std::stacktrace::current();
        // Adjust offsets as needed for your specific compiler's stack depth
        for (const auto& [idx, frame] : std::views::enumerate(trace)) {
            ss << "    " << idx << "#  " << frame << "\n";
        }
        return ss.str();
    }
public:
    tracable_runtime_error(std::string_view msg) : std::runtime_error(transform_message(msg)) {}
    tracable_runtime_error(const std::stringstream& msg) : std::runtime_error(transform_message(msg.str())) {}
};
#define my_error tracable_runtime_error

#else
// Fallback for Clang, MSVC, or older GCC
class fallback_runtime_error : public std::runtime_error {
public:
    fallback_runtime_error(std::string_view msg) : std::runtime_error(std::string(msg)) {}
    fallback_runtime_error(const std::stringstream& msg) : std::runtime_error(msg.str()) {}
};
#define my_error fallback_runtime_error
#endif


#define UNUSED(x) (void)(x)

#if defined(NDEBUG) || defined(IGNORE_NOT_IMPLEMENTED)
#define NOT_IMPLEMENTED()
#else
[[noreturn]] inline void NOT_IMPLEMENTED() { throw my_error("Called NOT_IMPLEMENTED"); }
#endif
