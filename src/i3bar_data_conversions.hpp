#ifndef I3NEOSTATUS_I3BAR_DATA_CONVERSIONS_HPP
#define I3NEOSTATUS_I3BAR_DATA_CONVERSIONS_HPP

#include "i3bar_data.hpp"

#include <string>
#include <vector>

namespace i3neostatus {
namespace i3bar_data {
namespace types {
template <typename t_value> std::string to_string(t_value value);
template <> std::string to_string<text_align>(const text_align value);
template <> std::string to_string<markup>(const markup value);

template <typename t_value> t_value from_string(const std::string &string);
template <typename t_value>
t_value from_string(const std::vector<std::string> &string);
template <>
click_modifiers from_string<click_modifiers>(const std::string &string);
template <>
click_modifiers
from_string<click_modifiers>(const std::vector<std::string> &string);
} // namespace types
} // namespace i3bar_data
} // namespace i3neostatus

#endif
