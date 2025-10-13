/**
 * @file xml.cpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief Helper implementation of xml data structures
 *
 * @date Created: 13. 10. 2025
 * @date Modified: 13. 10. 2025
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */
#include "xml.hpp"
#include <sstream>

namespace vkg_gen::xml {

    Error::Error(const std::string& data, const Position& pos, const char* action, const char* file_path, const std::string& msg,
        const int err_start, const int err_len, const int line_start, bool backwards) : std::runtime_error([&] {
            std::stringstream ss;
            ss << "Error occured when " << action << " at " << file_path << ':' << pos.line << ':' << pos.col << '\n' << msg << '\n';
            if (err_len == 0)
                return ss.str();

            sv line{ &data[line_start], data.end().base() };
            size_t new_size = line.find_first_of('\n');
            if (new_size != sv::npos)
                line = line.substr(0, new_size);

            int error_loc = err_start - line_start - backwards * err_len;
            ss << pos.line << " | " << line << '\n';
            ss << pos.line << " | " << std::string(error_loc, ' ') << std::string(err_len, '^') << "~~~here" << std::endl;
            return ss.str();
            }()) {};
}
