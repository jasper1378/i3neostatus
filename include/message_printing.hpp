#ifndef I3NEOSTATUS_MESSAGE_PRINTING_HPP
#define I3NEOSTATUS_MESSAGE_PRINTING_HPP

#include "program_constants.hpp"

#include <iostream>
#include <string_view>

namespace i3neostatus {
namespace message_printing {
void program_info(std::ostream &output_stream = std::cout);
void help(const std::string_view argv_0 = program_constants::k_name,
          std::ostream &output_stream = std::cout);
void version(std::ostream &output_stream = std::cout);
void error(const std::string_view error, bool exit = false,
           std::ostream &output_stream = std::cerr);
void error(const std::exception &exception, bool exit = false,
           std::ostream &output_stream = std::cerr);
} // namespace message_printing
} // namespace i3neostatus

#endif
