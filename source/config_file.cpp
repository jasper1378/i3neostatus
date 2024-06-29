#include "config_file.hpp"

#include "program_constants.hpp"
#include "theme.hpp"

#include "bits-and-bytes/constexpr_hash_string.hpp"
#include "bits-and-bytes/resolve_tilde.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

i3neostatus::config_file::error::error(const std::string &message,
                                       const std::string &file_path /*= ""*/,
                                       const long long pos_line /*= -1*/,
                                       const long long pos_char /*= -1*/)
    : base_t{((!file_path.empty())
                  ? (file_path +
                     ((pos_line != -1)
                          ? (m_k_separator_char + std::to_string(pos_line) +
                             ((pos_char != -1) ? (m_k_separator_char +
                                                  std::to_string(pos_char))
                                               : ("")))
                          : ("")) +
                     m_k_separator_char + m_k_whitespace_char + message)
                  : (message))} {}

i3neostatus::config_file::error::error(const char *message,
                                       const char *file_path /*= ""*/,
                                       const long long pos_line /*= -1*/,
                                       const long long pos_char /*= -1*/)
    : error{std::string{message}, std::string{file_path}, pos_line, pos_char} {}

i3neostatus::config_file::error::error(const error &other) : base_t{other} {}

i3neostatus::config_file::error::~error() {}

i3neostatus::config_file::error &
i3neostatus::config_file::error::operator=(const error &other) {
  if (this != &other) {
    base_t::operator=(other);
  }
  return *this;
}

const char *i3neostatus::config_file::error::what() const noexcept {
  return base_t::what();
}

i3neostatus::config_file::parsed
i3neostatus::config_file::read(const char *file_path) {
  return impl::read(impl::file_path::resolve(file_path));
}

i3neostatus::config_file::parsed
i3neostatus::config_file::read(const std::string &file_path) {
  return impl::read(impl::file_path::resolve(file_path));
}

i3neostatus::config_file::parsed
i3neostatus::config_file::read(const std::filesystem::path &file_path) {
  return impl::read(impl::file_path::resolve(file_path.string()));
}

i3neostatus::config_file::parsed i3neostatus::config_file::read() {
  static const std::string k_file_name_short{"config"};
  static const std::string k_file_name_long{program_constants::k_name +
                                            ".conf"};

  static const char *k_env_home{std::getenv("HOME")};
  if (k_env_home == nullptr) {
    throw error{"can't get 'HOME' environment variable"};
  }
  static const std::filesystem::path k_home_dir{k_env_home};

  static const char *k_env_xdg_config_home{std::getenv("XDG_CONFIG_HOME")};
  static const char *k_env_xdg_config_dirs{std::getenv("XDG_CONFIG_DIRS")};

  static const std::filesystem::path k_conf_path_1{
      (k_env_xdg_config_home != nullptr)
          ? (std::filesystem::path{k_env_xdg_config_home} /
             (program_constants::k_name + "/config"))
          : (k_home_dir /
             (".config/" + program_constants::k_name + "/config"))};
  static const std::filesystem::path k_conf_path_2{
      (k_env_xdg_config_dirs != nullptr)
          ? (std::filesystem::path{k_env_xdg_config_dirs} /
             (program_constants::k_name + "/config"))
          : (std::filesystem::path{"/etc/xdg/" + program_constants::k_name +
                                   "/config"})};
  static const std::filesystem::path k_conf_path_3{
      k_home_dir / ("." + program_constants::k_name + ".conf")};
  static const std::filesystem::path k_conf_path_4{
      "/etc/" + program_constants::k_name + ".conf"};

  if (std::filesystem::exists(k_conf_path_1)) {
    return config_file::read(k_conf_path_1);
  } else if (std::filesystem::exists(k_conf_path_2)) {
    return config_file::read(k_conf_path_2);
  } else if (std::filesystem::exists(k_conf_path_3)) {
    return config_file::read(k_conf_path_3);
  } else if (std::filesystem::exists(k_conf_path_4)) {
    return config_file::read(k_conf_path_4);
  } else {
    throw error{"can't find configuration file"};
  }
}

libconfigfile::node_ptr<libconfigfile::map_node>
i3neostatus::config_file::impl::libconfigfile_parse_file_wrapper(
    const std::string &file_path) {
  try {
    return libconfigfile::parse_file(file_path);
  } catch (const libconfigfile::syntax_error &ex) {
    throw error{ex.message(), file_path, ex.pos_line(), ex.pos_char()};
  } catch (const std::exception &ex) {
    throw error{ex.what(), file_path};
  }
}

i3neostatus::config_file::error
i3neostatus::config_file::impl::error_helpers::invalid_option(
    const std::string &file_path, const std::string &option_str) {
  return error{"invalid option: \"" + option_str + "\"", file_path};
}

i3neostatus::config_file::error
i3neostatus::config_file::impl::error_helpers::missing_option(
    const std::string &file_path, const std::string &option_str) {
  return error{"missing option: \"" + option_str + "\"", file_path};
}

i3neostatus::config_file::error
i3neostatus::config_file::impl::error_helpers::invalid_data_type_for(
    const std::string &file_path, const std::string &option_str,
    const std::string &valid_type) {
  return error{"invalid data type for: \"" + option_str + "\" (should be " +
                   valid_type + ")",
               file_path};
}

i3neostatus::config_file::error
i3neostatus::config_file::impl::error_helpers::invalid_data_type_in(
    const std::string &file_path, const std::string &option_str,
    const std::string &valid_type) {
  return error{"invalid data type in: \"" + option_str + "\" (should be " +
                   valid_type + ")",
               file_path};
}

i3neostatus::config_file::error
i3neostatus::config_file::impl::error_helpers::invalid_format_for(
    const std::string &file_path, const std::string &option_str,
    const std::string &valid_format) {
  return error{"invalid format for: \"" + option_str + "\" (should be " +
                   valid_format + ")",
               file_path};
}
std::string i3neostatus::config_file::impl::file_path::resolve(
    const std::string &file_path) {
  if (bits_and_bytes::resolve_tilde::would_resolve_tilde(file_path)) {
    return bits_and_bytes::resolve_tilde::resolve_tilde(file_path);
  } else {
    return file_path;
  }
}

std::string
i3neostatus::config_file::impl::file_path::resolve(std::string &&file_path) {
  if (bits_and_bytes::resolve_tilde::would_resolve_tilde(file_path)) {
    return bits_and_bytes::resolve_tilde::resolve_tilde(file_path);
  } else {
    return file_path;
  }
}

i3neostatus::config_file::parsed
i3neostatus::config_file::impl::read(const std::string &file_path) {
  if (!std::filesystem::exists(file_path)) {
    throw error{"configuration file does not exist", file_path};
  } else {
    libconfigfile::node_ptr<libconfigfile::map_node> libcf_parsed{
        libconfigfile_parse_file_wrapper(file_path)};
    parsed parsed{};

    for (auto ptr{libcf_parsed->begin()}; ptr != libcf_parsed->end(); ++ptr) {
      switch (bits_and_bytes::constexpr_hash_string::hash(ptr->first)) {
      case bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_general): {
        parsed.general =
            section_handlers::general(file_path, std::move(ptr->second));
      } break;
      case bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme): {
        parsed.theme =
            section_handlers::theme(file_path, std::move(ptr->second));
      } break;
      case bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_modules): {
        parsed.modules =
            section_handlers::modules(file_path, std::move(ptr->second));
      } break;
      default: {
        throw error_helpers::invalid_option(file_path, ptr->first);
      } break;
      }
    }

    return parsed;
  }
}

decltype(i3neostatus::config_file::parsed::general)
i3neostatus::config_file::impl::section_handlers::general(
    const std::string &file_path,
    libconfigfile::node_ptr<libconfigfile::node, true> &&ptr) {
  decltype(parsed::general) ret_val{.custom_separators{false}};

  if (ptr->get_node_type() == libconfigfile::node_type::Map) {
    libconfigfile::node_ptr<libconfigfile::map_node> ptr1_map{
        libconfigfile::node_ptr_cast<libconfigfile::map_node>(std::move(ptr))};
    for (auto ptr2{ptr1_map->begin()}; ptr2 != ptr1_map->end(); ++ptr2) {
      switch (bits_and_bytes::constexpr_hash_string::hash(ptr2->first)) {
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_general_custom_separators)): {
        if (ptr2->second->get_node_type() ==
            libconfigfile::node_type::Integer) {
          ret_val.custom_separators =
              static_cast<bool>(libconfigfile::node_to_base(std::move(
                  *libconfigfile::node_ptr_cast<libconfigfile::integer_node>(
                      std::move(ptr2->second)))));
        } else {
          throw error_helpers::invalid_data_type_for(
              file_path,
              (constants::option_str::k_general +
               error_helpers::k_nested_option_separator_char + ptr2->first),
              libconfigfile::node_type_to_str(
                  libconfigfile::node_type::Integer));
        }
      } break;
      default: {
        throw error_helpers::invalid_option(
            file_path,
            (constants::option_str::k_general +
             error_helpers::k_nested_option_separator_char + ptr2->first));
      } break;
      }
    }
  } else {
    throw error_helpers::invalid_data_type_for(
        file_path, constants::option_str::k_general,
        libconfigfile::node_type_to_str(libconfigfile::node_type::Map));
  }

  return ret_val;
}

decltype(i3neostatus::config_file::parsed::theme)
i3neostatus::config_file::impl::section_handlers::theme(
    const std::string &file_path,
    libconfigfile::node_ptr<libconfigfile::node, true> &&ptr) {
  decltype(parsed::theme) ret_val{theme::k_default};

  if (ptr->get_node_type() == libconfigfile::node_type::Map) {
    libconfigfile::node_ptr<libconfigfile::map_node> ptr_map{
        libconfigfile::node_ptr_cast<libconfigfile::map_node>(std::move(ptr))};
    for (auto ptr2{ptr_map->begin()}; ptr2 != ptr_map->end(); ++ptr2) {
      switch (bits_and_bytes::constexpr_hash_string::hash(ptr2->first)) {
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_idle_color_foreground)): {
        ret_val.state_dependent_color_foreground[static_cast<std::size_t>(
            block_state::idle)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_idle_color_background)): {
        ret_val.state_dependent_color_background[static_cast<std::size_t>(
            block_state::idle)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;

      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_idle_color_border)): {
        ret_val.state_dependent_color_border[static_cast<std::size_t>(
            block_state::idle)] =
            theme_helpers::read_border_color(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_info_color_foreground)): {
        ret_val.state_dependent_color_foreground[static_cast<std::size_t>(
            block_state::info)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_info_color_background)): {
        ret_val.state_dependent_color_background[static_cast<std::size_t>(
            block_state::info)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_info_color_border)): {
        ret_val.state_dependent_color_border[static_cast<std::size_t>(
            block_state::info)] =
            theme_helpers::read_border_color(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_good_color_foreground)): {
        ret_val.state_dependent_color_foreground[static_cast<std::size_t>(
            block_state::good)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_good_color_background)): {
        ret_val.state_dependent_color_background[static_cast<std::size_t>(
            block_state::good)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_good_color_border)): {
        ret_val.state_dependent_color_border[static_cast<std::size_t>(
            block_state::good)] =
            theme_helpers::read_border_color(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_warning_color_foreground)): {
        ret_val.state_dependent_color_foreground[static_cast<std::size_t>(
            block_state::warning)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_warning_color_background)): {
        ret_val.state_dependent_color_background[static_cast<std::size_t>(
            block_state::warning)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_warning_color_border)): {
        ret_val.state_dependent_color_border[static_cast<std::size_t>(
            block_state::warning)] =
            theme_helpers::read_border_color(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_critical_color_foreground)): {
        ret_val.state_dependent_color_foreground[static_cast<std::size_t>(
            block_state::critical)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_critical_color_background)): {
        ret_val.state_dependent_color_background[static_cast<std::size_t>(
            block_state::critical)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_critical_color_border)): {
        ret_val.state_dependent_color_border[static_cast<std::size_t>(
            block_state::critical)] =
            theme_helpers::read_border_color(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_error_color_foreground)): {
        ret_val.state_dependent_color_foreground[static_cast<std::size_t>(
            block_state::error)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_error_color_background)): {
        ret_val.state_dependent_color_background[static_cast<std::size_t>(
            block_state::error)] =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_error_color_border)): {
        ret_val.state_dependent_color_border[static_cast<std::size_t>(
            block_state::error)] =
            theme_helpers::read_border_color(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_alternating_tint_color_foreground)): {
        ret_val.alternating_tint_color_foreground =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_alternating_tint_color_background)): {
        ret_val.alternating_tint_color_background =
            theme_helpers::read_color<false>(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_alternating_tint_color_border)): {
        ret_val.alternating_tint_color_border =
            theme_helpers::read_border_color(file_path, ptr2->second,
                                             ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_middle_sequence)): {
        ret_val.separator_middle_sequence =
            theme_helpers::read_separator_sequence(file_path, ptr2->second,
                                                   ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_middle_color_foreground)): {
        ret_val.separator_middle_color_foreground =
            theme_helpers::read_separator_color<theme::separator_type::middle>(
                file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_middle_color_background)): {
        ret_val.separator_middle_color_background =
            theme_helpers::read_separator_color<theme::separator_type::middle>(
                file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_begin_sequence)): {
        ret_val.separator_begin_sequence =
            theme_helpers::read_separator_sequence(file_path, ptr2->second,
                                                   ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_begin_color_foreground)): {
        ret_val.separator_begin_color_foreground =
            theme_helpers::read_separator_color<theme::separator_type::begin>(
                file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_begin_color_background)): {
        ret_val.separator_begin_color_background =
            theme_helpers::read_separator_color<theme::separator_type::begin>(
                file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_end_sequence)): {
        ret_val.separator_end_sequence = theme_helpers::read_separator_sequence(
            file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_end_color_foreground)): {
        ret_val.separator_end_color_foreground =
            theme_helpers::read_separator_color<theme::separator_type::end>(
                file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_separator_end_color_background)): {
        ret_val.separator_end_color_background =
            theme_helpers::read_separator_color<theme::separator_type::end>(
                file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_border_width_top)): {
        ret_val.border_width_top = theme_helpers::read_border_width(
            file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_border_width_right)): {
        ret_val.border_width_right = theme_helpers::read_border_width(
            file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_border_width_bottom)): {
        ret_val.border_width_bottom = theme_helpers::read_border_width(
            file_path, ptr2->second, ptr2->first);
      } break;
      case (bits_and_bytes::constexpr_hash_string::hash(
          constants::option_str::k_theme_border_width_left)): {
        ret_val.border_width_left = theme_helpers::read_border_width(
            file_path, ptr2->second, ptr2->first);
      } break;
      default: {
        throw error_helpers::invalid_option(
            file_path,
            (constants::option_str::k_theme +
             error_helpers::k_nested_option_separator_char + ptr2->first));
      } break;
      }
    }
  } else {
    throw error_helpers::invalid_data_type_for(
        file_path, constants::option_str::k_theme,
        libconfigfile::node_type_to_str(libconfigfile::node_type::Map));
  }

  return ret_val;
}

std::variant<i3neostatus::theme::color,
             i3neostatus::theme::special_border_color>
i3neostatus::config_file::impl::section_handlers::theme_helpers::
    read_border_color(const std::string &file_path,
                      libconfigfile::node_ptr<libconfigfile::node> &&ptr,
                      const std::string &option_str) {
  return read_color<true, theme::special_border_color>(
      file_path, std::move(ptr), option_str,
      [&file_path,
       &option_str](const std::string &value) -> theme::special_border_color {
        static constexpr std::string k_special_str_foreground{"foreground"};
        static constexpr std::string k_special_str_background{"background"};
        switch (bits_and_bytes::constexpr_hash_string::hash(value)) {
        case (bits_and_bytes::constexpr_hash_string::hash(
            k_special_str_foreground)): {
          return theme::special_border_color::foreground;
        } break;
        case (bits_and_bytes::constexpr_hash_string::hash(
            k_special_str_background)): {
          return theme::special_border_color::background;
        } break;
        default: {
          throw error_helpers::invalid_format_for(
              file_path,
              (constants::option_str::k_theme +
               error_helpers::k_nested_option_separator_char + option_str),
              constants::error_str::k_format_color_or_special_str);
        } break;
        }
      });
}

std::string i3neostatus::config_file::impl::section_handlers::theme_helpers::
    read_separator_sequence(const std::string &file_path,
                            libconfigfile::node_ptr<libconfigfile::node> &&ptr,
                            const std::string &option_str) {
  if (ptr->get_node_type() == libconfigfile::node_type::String) {
    return libconfigfile::node_to_base(
        std::move(*libconfigfile::node_ptr_cast<libconfigfile::string_node>(
            std::move(ptr))));
  } else {
    throw error_helpers::invalid_data_type_for(
        file_path,
        (constants::option_str::k_theme +
         error_helpers::k_nested_option_separator_char + option_str),
        libconfigfile::node_type_to_str(libconfigfile::node_type::String));
  }
}

i3neostatus::theme::pixel_count_t
i3neostatus::config_file::impl::section_handlers::theme_helpers::
    read_border_width(const std::string &file_path,
                      libconfigfile::node_ptr<libconfigfile::node> &&ptr,
                      const std::string &option_str) {
  if (ptr->get_node_type() == libconfigfile::node_type::Integer) {
    libconfigfile::integer_node::base_t value{libconfigfile::node_to_base(
        std::move(*libconfigfile::node_ptr_cast<libconfigfile::integer_node>(
            std::move(ptr))))};
    if ((value <= std::numeric_limits<theme::pixel_count_t>::max()) &&
        (value >= std::numeric_limits<theme::pixel_count_t>::lowest())) {
      return static_cast<theme::pixel_count_t>(value);
    } else {
      throw error_helpers::invalid_range_for(
          file_path,
          (constants::option_str::k_theme +
           error_helpers::k_nested_option_separator_char + option_str),
          constants::error_str::k_range_pixel_count);
    }
  } else {
    throw error_helpers::invalid_data_type_for(
        file_path,
        (constants::option_str::k_theme +
         error_helpers::k_nested_option_separator_char + option_str),
        libconfigfile::node_type_to_str(libconfigfile::node_type::Integer));
  }
}

decltype(i3neostatus::config_file::parsed::modules)
i3neostatus::config_file::impl::section_handlers::modules(
    const std::string &file_path,
    libconfigfile::node_ptr<libconfigfile::node, true> &&ptr) {
  decltype(parsed::modules) ret_val{};

  if (ptr->get_node_type() == libconfigfile::node_type::Array) {
    libconfigfile::node_ptr<libconfigfile::array_node> ptr1_array{
        libconfigfile::node_ptr_cast<libconfigfile::array_node>(
            std::move(ptr))};
    ret_val.resize(ptr1_array->size());
    for (auto ptr2{ptr1_array->begin()}; ptr2 != ptr1_array->end(); ++ptr2) {
      if ((*ptr2)->get_node_type() == libconfigfile::node_type::Map) {
        libconfigfile::node_ptr<libconfigfile::map_node> ptr2_map{
            libconfigfile::node_ptr_cast<libconfigfile::map_node>(
                std::move(*ptr2))};
        for (auto ptr3{ptr2_map->begin()}; ptr3 != ptr2_map->end(); ++ptr3) {
          switch (bits_and_bytes::constexpr_hash_string::hash(ptr3->first)) {
          case (bits_and_bytes::constexpr_hash_string::hash(
              constants::option_str::k_modules_path)): {
            if (ptr3->second->get_node_type() ==
                libconfigfile::node_type::String) {
              std::string module_file_path{
                  libconfigfile::node_to_base(std::move(
                      *libconfigfile::node_ptr_cast<libconfigfile::string_node>(
                          std::move(ptr3->second))))};
              if (module_file_path.front() ==
                  constants::misc::k_builtin_file_prefix_remove) {
                ret_val[std::distance(ptr1_array->begin(), ptr2)].file_path =
                    file_path::resolve(
                        constants::misc::k_builtin_module_path /
                        (module_file_path.substr(1) +
                         constants::misc::k_builtin_module_file_suffix_add));
              } else {
                ret_val[std::distance(ptr1_array->begin(), ptr2)].file_path =
                    file_path::resolve(std::move(module_file_path));
              }
            } else {
              throw error_helpers::invalid_data_type_for(
                  file_path,
                  (constants::option_str::k_modules +
                   error_helpers::k_nested_option_separator_char + ptr3->first),
                  libconfigfile::node_type_to_str(
                      libconfigfile::node_type::String));
            }
          } break;
          case (bits_and_bytes::constexpr_hash_string::hash(
              constants::option_str::k_modules_config)): {
            if (ptr3->second->get_node_type() ==
                libconfigfile::node_type::Map) {
              ret_val[std::distance(ptr1_array->begin(), ptr2)].config =
                  std::move(
                      *libconfigfile::node_ptr_cast<libconfigfile::map_node>(
                          std::move(ptr3->second)));
            } else {
              throw error_helpers::invalid_data_type_for(
                  file_path,
                  (constants::option_str::k_modules +
                   error_helpers::k_nested_option_separator_char + ptr3->first),
                  libconfigfile::node_type_to_str(
                      libconfigfile::node_type::Map));
            }
          } break;
          default: {
            throw error_helpers::invalid_option(
                file_path,
                (constants::option_str::k_modules +
                 error_helpers::k_nested_option_separator_char + ptr3->first));
          } break;
          }
        }
        if (ret_val[std::distance(ptr1_array->begin(), ptr2)]
                .file_path.empty()) {
          throw error_helpers::missing_option(
              file_path, (constants::option_str::k_modules +
                          error_helpers::k_nested_option_separator_char +
                          constants::option_str::k_modules_path));
        }
      } else {
        throw error_helpers::invalid_data_type_in(
            file_path, constants::option_str::k_modules,
            libconfigfile::node_type_to_str(libconfigfile::node_type::Map));
      }
    }
  } else {
    throw error_helpers::invalid_data_type_for(
        file_path, constants::option_str::k_modules,
        libconfigfile::node_type_to_str(libconfigfile::node_type::Array));
  }

  return ret_val;
}
