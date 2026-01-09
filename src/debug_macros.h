#pragma once

// THIS FILE WILL BE DELETED IN FUTURE
// It is here only for debugging purposes

#include <assert.h>


#if	defined(NDEBUG) || defined(IGNORE_NOT_IMPLEMENTED)
#define NOT_IMPLEMENTED()
#define UNUSED(x)
#define my_error std::runtime_error
#else

// Using gcc specific experimental library, but it should be part of C++23
#include <stacktrace>

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
