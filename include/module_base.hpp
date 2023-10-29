#ifndef MODULE_BASE_HPP
#define MODULE_BASE_HPP

#include "module_api.hpp"

#include <string>

class module_base {
public:
  virtual ~module_base();

public:
  virtual module_api::config_out init(module_api &api,
                                      module_api::config_in &&config) = 0;

  virtual void run() = 0;

  virtual void term() = 0;

  virtual void on_click_event(module_api::click_event &&click_event) = 0;
};

#endif
