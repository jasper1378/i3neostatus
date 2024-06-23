#include "message_printing.hpp"

#include "program_constants.hpp"

#include <iostream>
#include <string_view>

void i3neostatus::message_printing::program_info(
    std::ostream &output_stream /*= std::cout*/) {
  output_stream << program_constants::k_name << ' '
                << program_constants::k_version << " © "
                << program_constants::k_year << ' '
                << program_constants::k_authors << '\n';
}

void i3neostatus::message_printing::help(
    const std::string_view argv_0 /*= program_constants::k_name*/,
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
