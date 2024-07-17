#include "plugin_error.hpp"

#include "plugin_id.hpp"

#include <exception>
#include <stdexcept>
#include <string>

i3neostatus::plugin_error::plugin_error(const plugin_id::type id,
                                        const std::string &name,
                                        const std::string &file_path,
                                        const std::string &what_arg)
    : base_t{"error in plugin; id = \"" + std::to_string(id) + "\", name = \"" +
             name + "\", file path = \"" + file_path + "\"; \"" + what_arg +
             "\""} {}

i3neostatus::plugin_error::plugin_error(const plugin_id::type id,
                                        const char *name, const char *file_path,
                                        const char *what_arg)
    : plugin_error{id, std::string{name}, std::string{file_path},
                   std::string{what_arg}} {}

i3neostatus::plugin_error::plugin_error(const plugin_error &other)
    : base_t{other} {}

i3neostatus::plugin_error::~plugin_error() {}

i3neostatus::plugin_error &
i3neostatus::plugin_error::operator=(const plugin_error &other) {
  if (this != &other) {
    base_t::operator=(other);
  }
  return *this;
}

const char *i3neostatus::plugin_error::what() const noexcept {
  return base_t::what();
}
