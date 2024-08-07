#ifndef I3NEOSTATUS_CONFIG_FILE_HPP
#define I3NEOSTATUS_CONFIG_FILE_HPP

#include "theme.hpp"

#include "bits-and-bytes/constexpr_hash_string.hpp"
#include "libconfigfile/libconfigfile.hpp"

#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace i3neostatus {

namespace config_file {
class error : public std::runtime_error {
private:
  using base_t = std::runtime_error;

private:
  static constexpr char m_k_separator_char{':'};
  static constexpr char m_k_whitespace_char{' '};

public:
  error(const std::string &message, const std::string &path = "",
        const long long pos_line = -1, const long long pos_char = -1);
  error(const char *message, const char *path = "",
        const long long pos_line = -1, const long long pos_char = -1);
  error(const error &other);

public:
  virtual ~error() override;

public:
  error &operator=(const error &other);

public:
  virtual const char *what() const noexcept override;
};

struct parsed {
  struct general {
    bool custom_separators;
  };

  struct plugin {
    std::variant<std::filesystem::path, std::string> path_or_name;
    libconfigfile::map_node config;
  };

  struct general general;
  struct theme::theme theme;
  std::vector<plugin> plugins;
};

parsed read(const char *path);
parsed read(const std::string &path);
parsed read(const std::filesystem::path &path);
parsed read();

bool is_builtin_plugin(const std::string &path_or_name);
std::string builtin_plugin_name(const std::string &name);
std::filesystem::path builtin_plugin_path(const std::string &name);

namespace impl {
parsed read(const std::string &path);

libconfigfile::node_ptr<libconfigfile::map_node>
libconfigfile_parse_file_wrapper(const std::string &path);

namespace constants {
namespace misc {
static const std::filesystem::path k_builtin_plugin_path{AM_PKGLIBDIR
                                                         "/plugins"};
static const std::filesystem::path k_builtin_theme_path{AM_PKGDATADIR
                                                        "/themes"};
static constexpr char k_builtin_file_suffix_remove{'_'};
static constexpr std::string k_builtin_plugin_file_suffix_add{".so"};
static constexpr std::string k_builtin_theme_file_suffix_add{".conf"};
} // namespace misc

namespace option_str {
static constexpr std::string k_general{"general"};
static constexpr std::string_view k_general_custom_separators{
    "custom_separators"};

static constexpr std::string k_theme{"theme"};
static constexpr std::string_view k_theme_idle_color_foreground{
    "idle_color_foreground"};
static constexpr std::string_view k_theme_idle_color_background{
    "idle_color_background"};
static constexpr std::string_view k_theme_idle_color_border{
    "idle_color_border"};
static constexpr std::string_view k_theme_info_color_foreground{
    "info_color_foreground"};
static constexpr std::string_view k_theme_info_color_background{
    "info_color_background"};
static constexpr std::string_view k_theme_info_color_border{
    "info_color_border"};
static constexpr std::string_view k_theme_good_color_foreground{
    "good_color_foreground"};
static constexpr std::string_view k_theme_good_color_background{
    "good_color_background"};
static constexpr std::string_view k_theme_good_color_border{
    "good_color_border"};
static constexpr std::string_view k_theme_warning_color_foreground{
    "warning_color_foreground"};
static constexpr std::string_view k_theme_warning_color_background{
    "warning_color_background"};
static constexpr std::string_view k_theme_warning_color_border{
    "warning_color_border"};
static constexpr std::string_view k_theme_critical_color_foreground{
    "critical_color_foreground"};
static constexpr std::string_view k_theme_critical_color_background{
    "critical_color_background"};
static constexpr std::string_view k_theme_critical_color_border{
    "critical_color_border"};
static constexpr std::string_view k_theme_error_color_foreground{
    "error_color_foreground"};
static constexpr std::string_view k_theme_error_color_background{
    "error_color_background"};
static constexpr std::string_view k_theme_error_color_border{
    "error_color_border"};
static constexpr std::string_view k_theme_alternating_tint_color_foreground{
    "alternating_tint_color_foreground"};
static constexpr std::string_view k_theme_alternating_tint_color_background{
    "alternating_tint_color_background"};
static constexpr std::string_view k_theme_alternating_tint_color_border{
    "alternating_tint_color_border"};
static constexpr std::string_view k_theme_separator_middle_sequence{
    "separator_middle_sequence"};
static constexpr std::string_view k_theme_separator_middle_color_foreground{
    "separator_middle_color_foreground"};
static constexpr std::string_view k_theme_separator_middle_color_background{
    "separator_middle_color_background"};
static constexpr std::string_view k_theme_separator_begin_sequence{
    "separator_begin_sequence"};
static constexpr std::string_view k_theme_separator_begin_color_foreground{
    "separator_begin_color_foreground"};
static constexpr std::string_view k_theme_separator_begin_color_background{
    "separator_begin_color_background"};
static constexpr std::string_view k_theme_separator_end_sequence{
    "separator_end_sequence"};
static constexpr std::string_view k_theme_separator_end_color_foreground{
    "separator_end_color_foreground"};
static constexpr std::string_view k_theme_separator_end_color_background{
    "separator_end_color_background"};
static constexpr std::string_view k_theme_border_width_top{"border_width_top"};
static constexpr std::string_view k_theme_border_width_right{
    "border_width_right"};
static constexpr std::string_view k_theme_border_width_bottom{
    "border_width_bottom"};
static constexpr std::string_view k_theme_border_width_left{
    "border_width_left"};

static constexpr std::string k_plugins{"plugins"};
static constexpr std::string k_plugins_path_or_name{"path_or_name"};
static constexpr std::string k_plugins_config{"config"};
} // namespace option_str

namespace error_str {
static constexpr std::string k_format_color{
    (std::is_same_v<theme::color, libconfigfile::color::rgba>)
        ? ("\"#RRGGBBA\"")
        : ("\"#RRGGBB\"")};
static const std::string k_format_color_or_special_str{k_format_color +
                                                       " or special string"};
static constexpr std::pair<theme::pixel_count_t, theme::pixel_count_t>
    k_range_pixel_count{std::numeric_limits<theme::pixel_count_t>::lowest(),
                        std::numeric_limits<theme::pixel_count_t>::max()};
} // namespace error_str
} // namespace constants

namespace error_helpers {
static constexpr char k_nested_option_separator_char{'/'};
error invalid_option(const std::string &path, const std::string &option_str);
error missing_option(const std::string &path, const std::string &option_str);
error invalid_data_type_for(const std::string &path,
                            const std::string &option_str,
                            const std::string &valid_type);
error invalid_data_type_in(const std::string &path,
                           const std::string &option_str,
                           const std::string &valid_type);
error invalid_format_for(const std::string &path, const std::string &option_str,
                         const std::string &valid_format);
template <typename T>
error invalid_range_for(const std::string &path, const std::string &option_str,
                        const std::pair<T, T> &valid_range) {
  return error{"invalid range for: \"" + option_str + "\" (should be " +
                   std::to_string(valid_range.first) + "-" +
                   std::to_string(valid_range.second) + ")",
               path};
}
} // namespace error_helpers

namespace path {
std::string resolve(const std::string &path);
} // namespace path

namespace section_handlers {
decltype(parsed::general)
general(const std::string &path,
        libconfigfile::node_ptr<libconfigfile::node, true> &&ptr);
namespace general_helpers {}

decltype(parsed::theme)
theme(const std::string &_path,
      libconfigfile::node_ptr<libconfigfile::node, true> &&ptr);
namespace theme_helpers {
template <bool or_special_str, typename t_special = void *,
          typename t_special_handler = void *>
std::conditional_t<or_special_str, std::variant<theme::color, t_special>,
                   theme::color>
read_color(const std::string &path,
           libconfigfile::node_ptr<libconfigfile::node> &&ptr,
           const std::string &option_str,
           t_special_handler special_handler = {})
  requires((!or_special_str) ||
           (std::invocable<t_special_handler, const std::string &> &&
            std::same_as<t_special, std::invoke_result_t<t_special_handler,
                                                         const std::string &>>))
{
  if (ptr->get_node_type() == libconfigfile::node_type::String) {
    libconfigfile::node_ptr<libconfigfile::string_node> ptr_string{
        libconfigfile::node_ptr_cast<libconfigfile::string_node>(
            std::move(ptr))};
    std::optional<theme::color> color{
        libconfigfile::color::from_string<theme::color>(
            ptr_string->data(), (ptr_string->data() + ptr_string->size()))};
    if (color.has_value()) {
      return *color;
    } else {
      if constexpr (or_special_str) {
        return special_handler(
            libconfigfile::node_to_base(std::move(*ptr_string)));
      } else {
        throw error_helpers::invalid_format_for(
            path,
            (constants::option_str::k_theme +
             error_helpers::k_nested_option_separator_char + option_str),
            constants::error_str::k_format_color);
      }
    }
  } else {
    throw error_helpers::invalid_data_type_for(
        path,
        (constants::option_str::k_theme +
         error_helpers::k_nested_option_separator_char + option_str),
        libconfigfile::node_type_to_str(libconfigfile::node_type::String));
  }
}

std::variant<theme::color, theme::special_border_color>
read_border_color(const std::string &path,
                  libconfigfile::node_ptr<libconfigfile::node> &&ptr,
                  const std::string &option_str);

template <theme::separator_type separator_type>
std::variant<theme::color,
             theme::impl::separator_type_enum_to_special_separator_color_enum_t<
                 separator_type>>
read_separator_color(const std::string &path,
                     libconfigfile::node_ptr<libconfigfile::node> &&ptr,
                     const std::string &option_str) {
  using special_separator_color =
      theme::impl::separator_type_enum_to_special_separator_color_enum_t<
          separator_type>;
  return read_color<true, special_separator_color>(
      path, std::move(ptr), option_str,
      [&path,
       &option_str](const std::string &value) -> special_separator_color {
        static constexpr std::string k_special_str_left{"left"};
        static constexpr std::string k_special_str_right{"right"};
        const auto throw_error{[&path, &option_str]() -> void {
          throw error_helpers::invalid_format_for(
              path,
              (constants::option_str::k_theme +
               error_helpers::k_nested_option_separator_char + option_str),
              constants::error_str::k_format_color_or_special_str);
        }};
        switch (bits_and_bytes::constexpr_hash_string::hash(value)) {
        case (
            bits_and_bytes::constexpr_hash_string::hash(k_special_str_left)): {
          if constexpr ((separator_type == theme::separator_type::middle ||
                         separator_type == theme::separator_type::end)) {
            return special_separator_color::left;
          } else {
            throw_error();
          }
        } break;
        case (
            bits_and_bytes::constexpr_hash_string::hash(k_special_str_right)): {
          if constexpr ((separator_type == theme::separator_type::middle) ||
                        (separator_type == theme::separator_type::begin)) {
            return special_separator_color::right;
          } else {
            throw_error();
          }
        } break;
        default: {
          // throw_error();
          throw int{};
        } break;
        }
      });
}

std::string
read_separator_sequence(const std::string &path,
                        libconfigfile::node_ptr<libconfigfile::node> &&ptr,
                        const std::string &option_str);

theme::pixel_count_t
read_border_width(const std::string &path,
                  libconfigfile::node_ptr<libconfigfile::node> &&ptr,
                  const std::string &option_str);

} // namespace theme_helpers

decltype(parsed::plugins)
plugins(const std::string &path,
        libconfigfile::node_ptr<libconfigfile::node, true> &&ptr);
namespace plugins_helpers {}
} // namespace section_handlers
} // namespace impl
} // namespace config_file
} // namespace i3neostatus
#endif
