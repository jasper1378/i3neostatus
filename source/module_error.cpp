#include "module_error.hpp"

#include "module_id.hpp"

#include <exception>
#include <stdexcept>
#include <string>

module_error::module_error(const module_id::type id, const std::string &name,
                           const std::string &file_path,
                           const std::string &what_arg)
    : base_t{"error in module; id = \"" + std::to_string(id) + "\", name = \"" +
             name + "\", file path = \"" + file_path + "\"; \"" + what_arg +
             "\""} {}

module_error::module_error(const module_id::type id, const char *name,
                           const char *file_path, const char *what_arg)
    : module_error{id, std::string{name}, std::string{file_path},
                   std::string{what_arg}} {}

module_error::module_error(const module_error &other) : base_t{other} {}

module_error::~module_error() {}

module_error &module_error::operator=(const module_error &other) {
  if (this != &other) {
    base_t::operator=(other);
  }
  return *this;
}

const char *module_error::what() const noexcept { return base_t::what(); }
