#include "plugin_error.hpp"

#include "plugin_id.hpp"

#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <variant>

i3neostatus::plugin_error::plugin_error(const plugin_id::type id,
                                        const std::filesystem::path &path,
                                        const std::string &what_arg)
    : base_t{do_ctor(id, path, what_arg)} {}

i3neostatus::plugin_error::plugin_error(const plugin_id::type id,
                                        const std::string &name,
                                        const std::string &what_arg)
    : base_t{do_ctor(id, name, what_arg)} {}

i3neostatus::plugin_error::plugin_error(
    const plugin_id::type id,
    const std::variant<std::filesystem::path, std::string> &path_or_name,
    const std::string &what_arg)
    : base_t{std::visit(
          [id, &what_arg](auto &&path_or_name) {
            return do_ctor(id, path_or_name, what_arg);
          },
          path_or_name)} {}

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

i3neostatus::plugin_error::base_t
i3neostatus::plugin_error::do_ctor(const plugin_id::type id,
                                   const std::filesystem::path &path,
                                   const std::string &what_arg) {
  return base_t{"error in plugin; id = \"" + std::to_string(id) +
                "\", path = \"" + path.string() + "\"; \"" + what_arg + "\""};
}

i3neostatus::plugin_error::base_t
i3neostatus::plugin_error::do_ctor(const plugin_id::type id,
                                   const std::string &name,
                                   const std::string &what_arg) {
  return base_t{"error in plugin; id = \"" + std::to_string(id) +
                "\", name = \"" + name + "\"; \"" + what_arg + "\""};
}
