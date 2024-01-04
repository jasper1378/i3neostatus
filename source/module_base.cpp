#include "module_base.hpp"

#include "module_api.hpp"

void module_base::on_click_event(module_api::click_event &&click_event) {
  (void)click_event;
}
