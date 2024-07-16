#include "module_id.hpp"

#include <charconv>
#include <string_view>

i3neostatus::module_id::type
i3neostatus::module_id::from_string(const std::string_view str) {
  type result{null};
  std::from_chars(str.data(), str.data() + str.size(), result);
  return result;
}
