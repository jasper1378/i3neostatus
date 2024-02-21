#include "i3bar_data_conversions.hpp"

#include "i3bar_data.hpp"
#include "misc.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

template <>
std::string i3neostatus::i3bar_data::types::to_string<
    i3neostatus::i3bar_data::types::text_align>(const text_align value) {
  static constexpr std::array<std::string,
                              static_cast<std::size_t>(text_align::max)>
      lut{"", "center", "right", "left"};
  return lut[static_cast<std::size_t>(value)];
}

template <>
std::string i3neostatus::i3bar_data::types::to_string<
    i3neostatus::i3bar_data::types::markup>(const markup value) {
  static constexpr std::array<std::string,
                              static_cast<std::size_t>(markup::max)>
      lut{"", "pango"};
  return lut[static_cast<std::size_t>(value)];
}

template <>
i3neostatus::i3bar_data::types::click_modifiers
i3neostatus::i3bar_data::types::from_string<
    i3neostatus::i3bar_data::types::click_modifiers>(
    const std::string &string) {
  static constexpr std::string str_mod1{"Mod1"};
  static constexpr std::string str_mod2{"Mod2"};
  static constexpr std::string str_mod3{"Mod3"};
  static constexpr std::string str_mod4{"Mod4"};
  static constexpr std::string str_mod5{"Mod5"};
  static constexpr std::string str_shift{"Shift"};
  static constexpr std::string str_control{"Control"};
  static constexpr std::string str_lock{"Lock"};

  switch (misc::constexpr_hash_string::hash(string)) {
  case misc::constexpr_hash_string::hash(str_mod1): {
    return click_modifiers::mod1;
  } break;
  case misc::constexpr_hash_string::hash(str_mod2): {
    return click_modifiers::mod2;
  } break;
  case misc::constexpr_hash_string::hash(str_mod3): {
    return click_modifiers::mod3;
  } break;
  case misc::constexpr_hash_string::hash(str_mod4): {
    return click_modifiers::mod4;
  } break;
  case misc::constexpr_hash_string::hash(str_mod5): {
    return click_modifiers::mod5;
  } break;
  case misc::constexpr_hash_string::hash(str_shift): {
    return click_modifiers::shift;
  } break;
  case misc::constexpr_hash_string::hash(str_control): {
    return click_modifiers::control;
  } break;
  case misc::constexpr_hash_string::hash(str_lock): {
    return click_modifiers::lock;
  } break;
  default: {
    return click_modifiers::none;
  } break;
  }
}

template <>
i3neostatus::i3bar_data::types::click_modifiers
i3neostatus::i3bar_data::types::from_string<
    i3neostatus::i3bar_data::types::click_modifiers>(
    const std::vector<std::string> &string) {
  click_modifiers ret_val{click_modifiers::none};

  for (std::size_t i{0}; i < string.size(); ++i) {
    ret_val |= from_string<click_modifiers>(string[i]);
  }

  return ret_val;
}
