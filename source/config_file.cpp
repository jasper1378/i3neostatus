#include "config_file.hpp"

#include "misc.hpp"
#include "program_constants.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
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
  return impl::read(impl::resolve_path(file_path));
}

i3neostatus::config_file::parsed
i3neostatus::config_file::read(const std::string &file_path) {
  return impl::read(impl::resolve_path(file_path));
}

i3neostatus::config_file::parsed
i3neostatus::config_file::read(const std::filesystem::path &file_path) {
  return impl::read(impl::resolve_path(file_path.string()));
}

i3neostatus::config_file::parsed i3neostatus::config_file::read() {
  static const std::string file_name_short{"config"};
  static const std::string file_name_long{
      i3neostatus::program_constants::g_k_name + ".conf"};

  static const char *env_home{std::getenv("HOME")};
  if (env_home == nullptr) {
    throw error{"can't get 'HOME' environment variable"};
  }
  static const std::filesystem::path home_dir{env_home};

  static const char *env_xdg_config_home{std::getenv("XDG_CONFIG_HOME")};
  static const char *env_xdg_config_dirs{std::getenv("XDG_CONFIG_DIRS")};

  static const std::filesystem::path conf_path_1{
      (env_xdg_config_home != nullptr)
          ? (std::filesystem::path{env_xdg_config_home} / "i3neostatus/config")
          : (home_dir / ".config/i3neostatus/config")};
  static const std::filesystem::path conf_path_2{
      (env_xdg_config_dirs != nullptr)
          ? (std::filesystem::path{env_xdg_config_dirs} / "i3neostatus/config")
          : ("/etc/xdg/i3neostatus/config")};
  static const std::filesystem::path conf_path_3{home_dir /
                                                 ".i3neostatus.conf"};
  static const std::filesystem::path conf_path_4{"/etc/i3neostatus.conf"};

  if (std::filesystem::exists(conf_path_1)) {
    return i3neostatus::config_file::read(conf_path_1);
  } else if (std::filesystem::exists(conf_path_2)) {
    return i3neostatus::config_file::read(conf_path_2);
  } else if (std::filesystem::exists(conf_path_3)) {
    return i3neostatus::config_file::read(conf_path_3);
  } else if (std::filesystem::exists(conf_path_4)) {
    return i3neostatus::config_file::read(conf_path_4);
  } else {
    throw error{"can't find configuration file"};
  }
}

std::string
i3neostatus::config_file::impl::resolve_path(const std::string &file_path) {
  if (i3neostatus::misc::resolve_tilde::would_resolve_tilde(file_path)) {
    return i3neostatus::misc::resolve_tilde::resolve_tilde(file_path);
  } else {
    return file_path;
  }
}

std::string
i3neostatus::config_file::impl::resolve_path(std::string &&file_path) {
  if (i3neostatus::misc::resolve_tilde::would_resolve_tilde(file_path)) {
    return i3neostatus::misc::resolve_tilde::resolve_tilde(file_path);
  } else {
    return file_path;
  }
}

i3neostatus::config_file::parsed
i3neostatus::config_file::impl::read(const std::string &file_path) {
  static const std::filesystem::path builtin_module_install_path{
      i3neostatus::program_constants::g_k_install_path / "lib"};
  static constexpr char builtin_module_prefix_remove{'_'};
  static constexpr std::string builtin_module_suffix_add{".so"};

  static constexpr std::string option_str_general{"general"};
  static constexpr std::string option_str_modules{"modules"};
  static constexpr std::string option_str_modules_path{"path"};
  static constexpr std::string option_str_modules_config{"config"};

  const auto libconfigfile_node_ptr_type_to_str{
      [](const libconfigfile::node_type type) -> std::string {
        switch (type) {
        case libconfigfile::node_type::String: {
          return "string";
        } break;
        case libconfigfile::node_type::Integer: {
          return "integer";
        } break;
        case libconfigfile::node_type::Float: {
          return "float";
        } break;
        case libconfigfile::node_type::Array: {
          return "array";
        } break;
        case libconfigfile::node_type::Map: {
          return "map";
        } break;
        default: {
          return "null";
        } break;
        };
      }};

  const auto error_helper_invalid_option{
      [&file_path](const std::string &option) -> error {
        return error{"invalid option: \"" + option + "\"", file_path};
      }};
  const auto error_helper_missing_option{
      [&file_path](const std::string &option) -> error {
        return error{"missing option: \"" + option + "\"", file_path};
      }};
  const auto error_helper_invalid_data_type_for{
      [&file_path, &libconfigfile_node_ptr_type_to_str](
          const std::string &option,
          const libconfigfile::node_type valid_type) -> error {
        return error{"invalid data type for: \"" + option + "\" (should be " +
                         libconfigfile_node_ptr_type_to_str(valid_type) + ")",
                     file_path};
      }};
  const auto error_helper_invalid_data_type_in{
      [&file_path, &libconfigfile_node_ptr_type_to_str](
          const std::string &option,
          const libconfigfile::node_type valid_type) -> error {
        return error{"invalid data type in: \"" + option + "\" (should be " +
                         libconfigfile_node_ptr_type_to_str(valid_type) + ")",
                     file_path};
      }};

  static constexpr char nested_option_separator_char{'/'};

  if (!std::filesystem::exists(file_path)) {
    throw error{"configuration file does not exist", file_path};
  } else {
    libconfigfile::node_ptr<libconfigfile::map_node> libcf_parsed{
        libcf_parse_file_wrapper(file_path)};
    parsed parsed{};

    for (auto ptr1{libcf_parsed->begin()}; ptr1 != libcf_parsed->end();
         ++ptr1) {
      if (ptr1->first == option_str_general) {
        if (ptr1->second->get_node_type() == libconfigfile::node_type::Map) {
          libconfigfile::node_ptr<libconfigfile::map_node> ptr1_map{
              libconfigfile::node_ptr_cast<libconfigfile::map_node>(
                  std::move(ptr1->second))};
          for (auto ptr2{ptr1_map->begin()}; ptr2 != ptr1_map->end(); ++ptr2) {
            parsed.general.dummy_TODO = 1378;
            // TODO handle stuff within "general"
          }
        } else {
          throw error_helper_invalid_data_type_for(
              ptr1->first, libconfigfile::node_type::Map);
        }
      } else if (ptr1->first == option_str_modules) {
        if (ptr1->second->get_node_type() == libconfigfile::node_type::Array) {
          libconfigfile::node_ptr<libconfigfile::array_node> ptr1_array{
              libconfigfile::node_ptr_cast<libconfigfile::array_node>(
                  std::move(ptr1->second))};
          parsed.modules.resize(ptr1_array->size());
          for (auto ptr2{ptr1_array->begin()}; ptr2 != ptr1_array->end();
               ++ptr2) {
            if ((*ptr2)->get_node_type() == libconfigfile::node_type::Map) {
              libconfigfile::node_ptr<libconfigfile::map_node> ptr2_map{
                  libconfigfile::node_ptr_cast<libconfigfile::map_node>(*ptr2)};
              for (auto ptr3{ptr2_map->begin()}; ptr3 != ptr2_map->end();
                   ++ptr3) {
                if (ptr3->first == option_str_modules_path) {
                  if (ptr3->second->get_node_type() ==
                      libconfigfile::node_type::String) {
                    std::string module_file_path{libconfigfile::node_to_base(
                        std::move(*libconfigfile::node_ptr_cast<
                                  libconfigfile::string_node>(
                            std::move(ptr3->second))))};
                    if (module_file_path.front() ==
                        builtin_module_prefix_remove) {
                      parsed.modules[std::distance(ptr1_array->begin(), ptr2)]
                          .file_path =
                          resolve_path(builtin_module_install_path /
                                       (module_file_path.substr(1) +
                                        builtin_module_suffix_add));
                    } else {
                      parsed.modules[std::distance(ptr1_array->begin(), ptr2)]
                          .file_path =
                          resolve_path(std::move(module_file_path));
                    }
                  } else {
                    throw error_helper_invalid_data_type_for(
                        (option_str_modules + nested_option_separator_char +
                         ptr3->first),
                        libconfigfile::node_type::String);
                  }
                } else if (ptr3->first == option_str_modules_config) {
                  if (ptr3->second->get_node_type() ==
                      libconfigfile::node_type::Map) {
                    parsed.modules[std::distance(ptr1_array->begin(), ptr2)]
                        .config = std::move(
                        *libconfigfile::node_ptr_cast<libconfigfile::map_node>(
                            std::move(ptr3->second)));
                  } else {
                    throw error_helper_invalid_data_type_for(
                        (option_str_modules + nested_option_separator_char +
                         ptr3->first),
                        libconfigfile::node_type::Map);
                  }
                } else {
                  throw error_helper_invalid_option(
                      (option_str_modules + nested_option_separator_char +
                       ptr3->first));
                }
              }
              if (parsed.modules[std::distance(ptr1_array->begin(), ptr2)]
                      .file_path.empty()) {
                throw error_helper_missing_option(
                    (option_str_modules + nested_option_separator_char +
                     option_str_modules_path));
              }
            } else {
              throw error_helper_invalid_data_type_in(
                  ptr1->first, libconfigfile::node_type::Map);
            }
          }
        } else {
          throw error_helper_invalid_data_type_for(
              ptr1->first, libconfigfile::node_type::Array);
        }
      } else {
        throw error_helper_invalid_option(ptr1->first);
      }
    }

    return parsed;
  }
}

libconfigfile::node_ptr<libconfigfile::map_node>
i3neostatus::config_file::impl::libcf_parse_file_wrapper(
    const std::string &file_path) {
  try {
    return libconfigfile::parse_file(file_path);
  } catch (const libconfigfile::syntax_error &ex) {
    throw error{ex.message(), file_path, ex.pos_line(), ex.pos_char()};
  } catch (const std::exception &ex) {
    throw error{ex.what(), file_path};
  }
}
