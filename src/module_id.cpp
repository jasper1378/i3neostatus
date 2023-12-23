#include "module_id.hpp"

#include <charconv>
#include <string>

module_id::type module_id::from_string(const std::string &str) {
  type result{null};
  std::from_chars(str.data(), str.data() + str.size(), result);
  return result;
}
