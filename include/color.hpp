#ifndef I3NEOSTATUS_COLOR_HPP
#define I3NEOSTATUS_COLOR_HPP

#include "define_enum_flag_operators.hpp"

#include <cstdint>
#include <string>

namespace i3neostatus {
namespace color {

using channel_t = uint8_t;

struct rgb {
  channel_t r;
  channel_t g;
  channel_t b;
};
struct rgba : public rgb {
  channel_t a;
};

enum class to_string_flags : unsigned int {
  none = 0b000,
  cap_digits = 0b001,
  no_prefix = 0b010,
  alpha_first = 0b100,
};
DEFINE_ENUM_FLAG_OPERATORS_FOR_TYPE(to_string_flags);

template <typename t_color>
concept concept_color =
    (std::same_as<t_color, rgb> || std::same_as<t_color, rgba>);

template <typename t_color>
  requires concept_color<t_color>
std::string::size_type to_string_buf_len(const to_string_flags flags) {
  static constexpr std::string::size_type channel_len{2};
  static constexpr std::string::size_type channel_cnt{3};
  static constexpr std::string::size_type prefix_len{1};

  std::string::size_type len{channel_len * channel_cnt};
  if (!(static_cast<bool>(flags & to_string_flags::no_prefix))) {
    len += prefix_len;
  }
  if constexpr (std::is_same_v<t_color, rgba>) {
    len += channel_len;
  }

  return len;
}

template <typename t_color>
  requires concept_color<t_color>
bool to_string(char *const str_begin, char *const str_end, const t_color &color,
               const to_string_flags flags = to_string_flags::none) {
  constexpr bool has_alpha{std::is_same_v<t_color, rgba>};

  bool cap_digits{static_cast<bool>(flags & to_string_flags::cap_digits)};
  bool no_prefix{static_cast<bool>(flags & to_string_flags::no_prefix)};
  bool alpha_first{static_cast<bool>(flags & to_string_flags::alpha_first)};

  const auto digit_to_char{[cap_digits](const channel_t digit) -> char {
    if (/*(digit >= 0) &&*/ (digit <= 9)) {
      return ('0' + digit);
    } else if ((digit >= 0xa) && (digit <= 0xf)) {
      return (((cap_digits) ? ('A') : ('a')) + (digit - 0xa));
    } else {
      return '\0';
    }
  }};
  const auto channel_to_chars{
      [&digit_to_char](const channel_t value, char *&cur_char) -> void {
        *cur_char = digit_to_char(value / 16);
        *(++cur_char) = digit_to_char(value % 16);
        ++cur_char;
      }};

  char *cur_char{str_begin};

  if (((str_begin == nullptr) || (str_end == nullptr)) ||
      ((str_end - str_begin) <
       (static_cast<std::ptrdiff_t>(to_string_buf_len<t_color>(flags))))) {
    return false;
  }

  if (!no_prefix) {
    *cur_char = '#';
    ++cur_char;
  }

  if constexpr (has_alpha) {
    if (alpha_first) {
      channel_to_chars(color.a, cur_char);
    }
    channel_to_chars(color.r, cur_char);
    channel_to_chars(color.g, cur_char);
    channel_to_chars(color.b, cur_char);
    if (!alpha_first) {
      channel_to_chars(color.a, cur_char);
    }
  } else {
    channel_to_chars(color.r, cur_char);
    channel_to_chars(color.g, cur_char);
    channel_to_chars(color.b, cur_char);
  }

  return true;
}

template <typename t_color>
  requires concept_color<t_color>
std::string to_string(const t_color &color,
                      const to_string_flags flags = to_string_flags::none) {
  std::string ret_val(to_string_buf_len<t_color>(flags), '0');
  to_string(ret_val.data(), (ret_val.data() + ret_val.size()), color, flags);
  return ret_val;
}

} // namespace color
} // namespace i3neostatus

#endif
