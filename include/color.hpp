#ifndef I3NEOSTATUS_COLOR_HPP
#define I3NEOSTATUS_COLOR_HPP

#include "define_enum_flag_operators.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>

namespace i3neostatus {
namespace color {
inline namespace rgb_model {

using channel_t = uint8_t;

struct rgb {
  channel_t r{0x00};
  channel_t g{0x00};
  channel_t b{0x00};
};

struct rgba : public rgb {
  channel_t a{0xff};
};

template <typename t_color>
concept concept_color =
    (std::same_as<t_color, rgb> || std::same_as<t_color, rgba>);

bool operator==(const rgb &color_1, const rgb &color_2);
bool operator==(const rgba &color_1, const rgba &color_2);

template <typename t_color_1, typename t_color_2>
  requires(concept_color<t_color_1> && concept_color<t_color_2>)
bool operator!=(const t_color_1 &color_1, const t_color_2 &color_2) {
  return !(color_1 == color_2);
}

enum class to_string_flags : unsigned int {
  none = 0b000,
  cap_digits = 0b001,
  no_prefix = 0b010,
  alpha_first = 0b100,
};
DEFINE_ENUM_FLAG_OPERATORS_FOR_TYPE(to_string_flags);

template <typename t_color_to, typename t_color_from>
  requires(concept_color<t_color_to> && concept_color<t_color_from>)
t_color_to convert(const t_color_from &color) {
  if constexpr (std::is_same_v<t_color_to, t_color_from>) {
    return color;
  } else if constexpr (std::is_base_of_v<t_color_to, t_color_from>) {
    return static_cast<t_color_to>(color);
  } else {
    t_color_to ret_val{color.r, color.g, color.b};
    return ret_val;
  }
}

template <typename t_color>
  requires concept_color<t_color>
std::optional<t_color> from_string(const char *const str_begin,
                                   const char *const str_end) {
  bool error{false};
  const char *cur_pos{str_begin};

  const auto read_channel{[str_end, &error, &cur_pos]() -> channel_t {
    const auto read_digit{[str_end, &error, &cur_pos]() -> channel_t {
      if (error) {
        return 0;
      }

      if (cur_pos == str_end) {
        error = true;
        return 0;
      } else {
        char cur_char{*(cur_pos++)};
        if (cur_char >= '0' && cur_char <= '9') {
          return (cur_char - '0');
        } else if (cur_char >= 'a' && cur_char <= 'f') {
          return (10 + (cur_char - 'a'));
        } else if (cur_char >= 'A' && cur_char <= 'F') {
          return (10 + (cur_char - 'A'));
        } else {
          error = true;
          return 0;
        }
      }
    }};

    channel_t ret_val{};

    ret_val = ((ret_val << 4) | (read_digit()));
    ret_val = ((ret_val << 4) | (read_digit()));

    return ret_val;
  }};

  t_color ret_val{};

  if (*cur_pos == '#') {
    ++cur_pos;
  }

  ret_val.r = read_channel();
  ret_val.g = read_channel();
  ret_val.b = read_channel();

  if constexpr (std::is_same_v<t_color, rgba>) {
    ret_val.a = read_channel();
  }

  return (error ? std::optional<t_color>(std::nullopt)
                : std::optional<t_color>(ret_val));
}

template <typename t_color>
  requires concept_color<t_color>
std::optional<t_color> from_string(const std::string &string) {
  return from_string<t_color>(string.data(), (string.data() + string.size()));
}

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

} // namespace rgb_model
} // namespace color
} // namespace i3neostatus

#endif
