#include "plugin_base.hpp"

#include "plugin_api.hpp"

void i3neostatus::plugin_base::on_click_event(
    plugin_api::click_event &&click_event) {
  (void)click_event;
}
