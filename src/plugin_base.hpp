#ifndef I3NEOSTATUS_PLUGIN_BASE_HPP
#define I3NEOSTATUS_PLUGIN_BASE_HPP

#include "plugin_api.hpp"

#include <string>

namespace i3neostatus {

class plugin_base {
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
