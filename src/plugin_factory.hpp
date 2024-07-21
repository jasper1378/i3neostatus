#ifndef I3NEOSTATUS_PLUGIN_FACTORY_HPP
#define I3NEOSTATUS_PLUGIN_FACTORY_HPP

#include "plugin_base.hpp"

#define I3NEOSTATUS_PLUGIN_FACTORY_DECLARE(plugin_)                            \
  extern "C" {                                                                 \
  i3neostatus::plugin_base *create();                                          \
  void destroy(i3neostatus::plugin_base *m);                                   \
  }
#define I3NEOSTATUS_PLUGIN_FACTORY_DEFINE(plugin_)                             \
  extern "C" {                                                                 \
  i3neostatus::plugin_base *create() { return new plugin_{}; }                 \
  void destroy(i3neostatus::plugin_base *m) { delete m; }                      \
  }

namespace i3neostatus {
namespace plugin_factory {
using create_func_t = plugin_base *();
using create_func_ptr_t = create_func_t *;
using destroy_func_t = void(plugin_base *);
using destroy_func_ptr_t = destroy_func_t *;

static constexpr std::string k_create_func_str{"create"};
static constexpr std::string k_destroy_func_str{"destroy"};

} // namespace plugin_factory
} // namespace i3neostatus

#endif
