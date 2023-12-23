#ifndef MISC_HPP
#define MISC_HPP

#include <string>

namespace misc {
static constexpr std::string g_k_program_name{"i3neostatus"};

namespace constexpr_minmax {
template <typename T> consteval auto max(const T &t) { return t; }
template <typename T, typename... U>
consteval auto max(const T &t, const U &...u) {
  return ((t > max(u...)) ? (t) : (max(u...)));
}
template <typename T> constexpr auto min(const T &t) { return t; }
template <typename T, typename... U>
constexpr auto min(const T &t, const U &...u) {
  return ((t < min(u...)) ? (t) : (min(u...)));
}
} // namespace constexpr_minmax

} // namespace misc

#endif
