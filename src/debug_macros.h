#pragma once

// THIS FILE WILL BE DELETED IN FUTURE
// It is here only for debugging purposes

#include <cassert>


#if	defined(NDEBUG) || defined(IGNORE_NOT_IMPLEMENTED)
#define NOT_IMPLEMENTED()
#define UNUSED(x)
#define my_error std::runtime_error
#elif !defined(__GNUC__) || defined(__clang__)
#include <stdexcept>
#include <sstream>
#include <string>
#include <string_view>

[[noreturn]] inline void NOT_IMPLEMENTED() { throw std::runtime_error("Called NOT_IMPLEMENTED"); }
#define UNUSED(x) (void)(x)
class fallback_runtime_error : public std::runtime_error {
public:
    fallback_runtime_error(std::string_view msg) : std::runtime_error(std::string(msg)) {}
    fallback_runtime_error(const std::stringstream& msg) : std::runtime_error(msg.str()) {}
};
#define my_error fallback_runtime_error

#else
// Using gcc specific experimental library, but it should be part of C++23
#include <stacktrace>
#include <stdexcept>
#include <source_location>
#include <iostream>
#include <sstream>
#include <ranges>

#define UNUSED(x) (void)(x)

[[noreturn]] inline void NOT_IMPLEMENTED() {
    std::cerr << "ERROR: Called NOT_IMPLEMENTED.\nStacktrace:\n";

    auto trace = std::stacktrace::current();
    auto frames = std::span(trace.begin() + 1, trace.end() - 4); // exclude this function and libc functions, that called main

    for (const auto& [idx, frame] : std::views::enumerate(frames)) {
        std::cerr << "    " << idx << "#  " << frame << "\n";

    }

    std::exit(1);
}

class tracable_runtime_error : public std::runtime_error {
    static std::string transform_message(std::string_view msg) {
        std::stringstream ss;
        ss << "ERROR: " << msg << "\nStacktrace:\n";

        auto trace = std::stacktrace::current();
        auto frames = std::span(trace.begin() + 1, trace.end() - 4); // exclude this function and libc functions, that called main

        for (const auto& [idx, frame] : std::views::enumerate(frames))
            ss << "    " << idx << "#  " << frame << "\n";

        return ss.str();
    }

public:
    tracable_runtime_error(std::string_view msg) : std::runtime_error(transform_message(msg)) {};
    tracable_runtime_error(const std::stringstream& msg) : std::runtime_error(transform_message(msg.str())) {};
};

#define my_error tracable_runtime_error

#endif
