#include "plugin_loader.hpp"

#include "config_file.hpp"
#include "dynamic_loader.hpp"
#include "plugin_base.hpp"
#include "plugin_error.hpp"
#include "plugin_factory.hpp"
#include "plugin_id.hpp"

#if !ENABLE_DYN_LOAD_PLUGIN_BUILTIN
#include "plugins_builtin.hpp"
#endif

#include "config.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>

const std::unordered_map<
    std::string, std::pair<i3neostatus::plugin_factory::create_func_ptr_t,
                           i3neostatus::plugin_factory::destroy_func_ptr_t>>
    i3neostatus::plugin_loader::m_k_plugin_factory_func_lut{
        {"test_plugin",
         {&plugins_builtin::test_plugin::create,
          &plugins_builtin::test_plugin::destroy}}};

i3neostatus::plugin_loader::plugin_loader(
    const std::variant<std::filesystem::path, std::string> &path_or_name,
    const plugin_id::type id)
    : m_instance{nullptr}, m_destroy_func{nullptr},
      m_dynamic_lib{std::nullopt} {
  switch (path_or_name.index()) {
  case 0: {
    ctor_dynamic(std::get<0>(path_or_name), id);
  } break;
  case 1: {
#if ENABLE_DYN_LOAD_PLUGIN_BUILTIN
    ctor_dynamic(config_file::builtin_plugin_path(std::get<1>(path_or_name)),
                 id);
#else
    ctor_nodynamic(std::get<1>(path_or_name), id);
#endif
  } break;
  }
  if (!m_instance) {
    throw plugin_error{id, path_or_name, "factory failed"};
  }
}

i3neostatus::plugin_loader::plugin_loader(plugin_loader &&other) noexcept
    : m_instance{other.m_instance}, m_destroy_func{other.m_destroy_func},
      m_dynamic_lib{std::move(other.m_dynamic_lib)} {
  other.m_instance = nullptr;
  other.m_destroy_func = nullptr;
}

i3neostatus::plugin_loader::~plugin_loader() {
  if (m_instance) {
    m_destroy_func(m_instance);
  }
}

i3neostatus::plugin_loader &
i3neostatus::plugin_loader::operator=(plugin_loader &&other) {
  if (this != &other) {
    m_instance = other.m_instance;
    m_destroy_func = other.m_destroy_func;
    other.m_instance = nullptr;
    other.m_destroy_func = nullptr;
    m_dynamic_lib = std::move(other.m_dynamic_lib);
  }
  return *this;
}

i3neostatus::plugin_base &i3neostatus::plugin_loader::get() {
  return *m_instance;
}

const i3neostatus::plugin_base &i3neostatus::plugin_loader::get() const {
  return *m_instance;
}

void i3neostatus::plugin_loader::ctor_dynamic(const std::filesystem::path &path,
                                              const plugin_id::type id) {
  if (!(std::filesystem::is_regular_file(path) ||
        std::filesystem::is_symlink(path))) {
    throw plugin_error{id, path, "file does not exist"};
  }

  m_dynamic_lib = dynamic_loader::lib{path, dynamic_loader::dlopen_flags::lazy};
  m_destroy_func = m_dynamic_lib->get_symbol<plugin_factory::destroy_func_t>(
      plugin_factory::k_destroy_func_str);
  m_instance = m_dynamic_lib->get_symbol<plugin_factory::create_func_t>(
      plugin_factory::k_create_func_str)();
}

#if !ENABLE_DYN_LOAD_PLUGIN_BUILTIN
void i3neostatus::plugin_loader::ctor_nodynamic(const std::string &name,
                                                const plugin_id::type id) {
  try {
    auto plugin_factory_func{m_k_plugin_factory_func_lut.at(name)};
    m_destroy_func = plugin_factory_func.second;
    m_instance = plugin_factory_func.first();
  } catch (const std::out_of_range &) {
    throw plugin_error{id, name, "name does not exist"};
  }
}
#endif
