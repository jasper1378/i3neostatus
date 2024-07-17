#include "plugin_id.hpp"

#include <charconv>
#include <string_view>

i3neostatus::plugin_id::type
i3neostatus::plugin_id::from_string(const std::string_view str) {
  type result{null};
  std::from_chars(str.data(), str.data() + str.size(), result);
  return result;
}
