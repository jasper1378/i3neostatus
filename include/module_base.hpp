#ifndef MODULE_BASE_HPP
#define MODULE_BASE_HPP

#include "module_api.hpp"

#include <string>

class module_base {
public:
  using allocator_func_t = module_base *();
  using allocator_func_ptr_t = allocator_func_t *;
  using deleter_func_t = void(module_base *);
  using deleter_func_ptr_t = deleter_func_t *;

  static constexpr std::string allocator_func_str{"allocator"};
  static constexpr std::string deleter_func_str{"deleter"};

public:
  virtual ~module_base() = default;

public:
  virtual module_api::config_out init(module_api &&api,
                                      module_api::config_in &&config) = 0;

  virtual void run() = 0;

  virtual void term() = 0;

  virtual void on_click_event(module_api::click_event &&click_event);
};

#endif
