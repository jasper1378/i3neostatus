#ifndef I3NEOSTATUS_PLUGIN_BASE_HPP
#define I3NEOSTATUS_PLUGIN_BASE_HPP

#include "plugin_api.hpp"

#include <string>

namespace i3neostatus {

class plugin_base {
public:
  using allocator_func_t = plugin_base *();
  using allocator_func_ptr_t = allocator_func_t *;
  using deleter_func_t = void(plugin_base *);
  using deleter_func_ptr_t = deleter_func_t *;

  static constexpr std::string allocator_func_str{"allocator"};
  static constexpr std::string deleter_func_str{"deleter"};

public:
  virtual ~plugin_base() = default;

public:
  virtual plugin_api::config_out init(plugin_api *api,
                                      plugin_api::config_in &&config) = 0;

  virtual void run() = 0;

  virtual void term() = 0;

  virtual void on_click_event(plugin_api::click_event &&click_event);
};

} // namespace i3neostatus
#endif
