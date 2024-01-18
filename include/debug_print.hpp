#ifndef I3NEOSTATUS_DEBUG_PRINT_HPP
#define I3NEOSTATUS_DEBUG_PRINT_HPP

#include <string>

namespace debug_print {
static constexpr std::string format_begin{"\033[1;31m"};
static constexpr std::string format_end{"\033[0m"};
} // namespace debug_print
