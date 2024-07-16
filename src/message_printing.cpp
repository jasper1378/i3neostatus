#include "message_printing.hpp"

#include "config.h"

#include <iostream>
#include <string_view>

void i3neostatus::message_printing::program_info(
    std::ostream &output_stream /*= std::cout*/) {
  output_stream << PACKAGE_NAME << ' ' << PACKAGE_VERSION << " © "
                << "2024" << ' ' << "Jasper Young" << '\n';
}

void i3neostatus::message_printing::help(
    const std::string_view argv_0 /*= PACKAGE_NAME*/,
    std::ostream &output_stream /*= std::cout*/) {
  program_info(output_stream);
  output_stream << "Syntax: " << argv_0 << " [-c <configfile>] [-h] [-v]\n";
}

void i3neostatus::message_printing::version(
    std::ostream &output_stream /*= std::cout*/) {
  program_info(output_stream);
}

void i3neostatus::message_printing::error(
    const std::string_view error, bool exit /*= false*/,
    std::ostream &output_stream /*= std::cerr*/) {
  output_stream << "Error: " << error << '\n';
  if (exit) {
    std::exit(EXIT_FAILURE);
  }
}

void i3neostatus::message_printing::error(
    const std::exception &exception, bool exit /*= false*/,
    std::ostream &output_stream /*= std::cerr*/) {
  error(exception.what(), exit, output_stream);
}
