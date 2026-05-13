/**
 * @file main.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Helper file with debug macros
 * @date Created: 15. 12. 2025
 * @date Modified: 16. 04. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once

 // THIS FILE MAY BE DELETED IN FUTURE
 // It is here only for debugging purposes

#include <cassert>
#include <cstdlib>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#if defined(__GNUC__) && !defined(__clang__) && __has_include(<stacktrace>)
#include <stacktrace>
#include <ranges>
#include <span>

class tracable_runtime_error : public std::runtime_error {
    static std::string transform_message(std::string_view msg) {
        std::stringstream ss;
        ss << "ERROR: " << msg << "\nStacktrace:\n";

        auto trace = std::stacktrace::current();
        auto frames = std::span(trace.begin() + 1, trace.end() - 4); // exclude this function and libc functions, that called main

        for (const auto& [idx, frame] : std::views::enumerate(frames)) {
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
    fallback_runtime_error(std::string&& msg) : std::runtime_error(std::move(msg)) {}
    fallback_runtime_error(const char* msg) : std::runtime_error(msg) {}

};
#define my_error fallback_runtime_error
#endif


#define UNUSED(x) (void)(x)

#if defined(NDEBUG)

// Release: abort quietly — safe inside noexcept, std::unreachable() lets the
// optimiser drop dead code after the call.
[[noreturn]] inline void NOT_IMPLEMENTED() { std::abort(); }
[[noreturn]] inline void UNREACHABLE() { std::unreachable(); }
#define UNREACHABLE_QUIET()
#else
#include <iostream>
[[noreturn]] inline void NOT_IMPLEMENTED() { throw my_error("Called NOT_IMPLEMENTED"); }
[[noreturn]] inline void UNREACHABLE(std::source_location loc = std::source_location::current()) {
    std::cerr << "Unreachable code reached at " << loc.file_name() << ":" << loc.line()
        << " in " << loc.function_name() << "\n";
    std::abort();
}
#define UNREACHABLE_QUIET() UNREACHABLE(std::source_location::current())

#endif
