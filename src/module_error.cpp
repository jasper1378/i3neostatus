#include "module_error.hpp"

#include "module_id.hpp"

#include <exception>
#include <stdexcept>
#include <string>

module_error::in::in(const module_id_t id, const std::string &name,
                     const std::string &file_path, const std::string &what_arg)
    : base_t{std::string{} + "error inside module; id = \"" +
             std::to_string(id) + "\", name = \"" + name +
             "\", file_path = \"" + file_path + "\"; \"" + what_arg + "\""} {}

module_error::in::in(const module_id_t id, const char *name,
                     const char *file_path, const char *what_arg)
    : base_t{std::string{} + "error inside module; id = \"" +
             std::to_string(id) + "\", name = \"" + name +
             "\", file_path = \"" + file_path + "\"; \"" + what_arg + "\""} {}

module_error::in::in(const in &other) : base_t{other} {}

module_error::in::~in() {}

module_error::in &module_error::in::operator=(const in &other) {
  if (this != &other) {
    base_t::operator=(other);
  }
  return *this;
}

const char *module_error::in::what() const noexcept { return base_t::what(); }

module_error::out::out(const module_id_t id, const std::string &name,
                       const std::string &file_path,
                       const std::string &what_arg)
    : base_t{std::string{} + "error outside module; id = \"" +
             std::to_string(id) + "\", name = \"" + name +
             "\", file_path = \"" + file_path + "\"; \"" + what_arg + "\""} {}

module_error::out::out(const module_id_t id, const char *name,
                       const char *file_path, const char *what_arg)
    : base_t{std::string{} + "error outside module; id = \"" +
             std::to_string(id) + "\", name = \"" + name +
             "\", file_path = \"" + file_path + "\"; \"" + what_arg + "\""} {}

module_error::out::out(const out &other) : base_t{other} {}

module_error::out::~out() {}

module_error::out &module_error::out::operator=(const out &other) {
  if (this != &other) {
    base_t::operator=(other);
  }
  return *this;
}

const char *module_error::out::what() const noexcept { return base_t::what(); }
