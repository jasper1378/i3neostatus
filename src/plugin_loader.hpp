#ifndef I3NEOSTATUS_PLUGIN_LOADER_HPP
#define I3NEOSTATUS_PLUGIN_LOADER_HPP

#include "config.h"

#include "dynamic_loader.hpp"
#include "plugin_base.hpp"
#include "plugin_factory.hpp"
#include "plugin_id.hpp"

#if !ENABLE_DYN_LOAD_PLUGIN_BUILTIN
#include "plugins_builtin.hpp"
#endif

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

namespace i3neostatus {
class plugin_loader {
private:
  plugin_base *m_instance;
  plugin_factory::destroy_func_ptr_t m_destroy_func;
  std::optional<dynamic_loader::lib> m_dynamic_lib;

private:
#if !ENABLE_DYN_LOAD_PLUGIN_BUILTIN
  static const std::unordered_map<std::string,
                                  std::pair<plugin_factory::create_func_ptr_t,
                                            plugin_factory::destroy_func_ptr_t>>
      m_k_plugin_factory_func_lut;
#endif

public:
  plugin_loader(
      const std::variant<std::filesystem::path, std::string> &path_or_name,
      const plugin_id::type id);
  plugin_loader(plugin_loader &&other) noexcept;
  plugin_loader(const plugin_loader &other) = delete;

public:
  ~plugin_loader();

public:
  plugin_loader &operator=(plugin_loader &&other);
  plugin_loader &operator=(const plugin_loader &&other) = delete;

public:
  plugin_base &get();
  const plugin_base &get() const;

private:
  void ctor_dynamic(const std::filesystem::path &path,
                    const plugin_id::type id);
#if !ENABLE_DYN_LOAD_PLUGIN_BUILTIN
  void ctor_nodynamic(const std::string &name, const plugin_id::type id);
#endif
};
} // namespace i3neostatus

#endif
