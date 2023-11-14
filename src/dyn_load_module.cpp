#include "dyn_load_module.hpp"

#include "dyn_load_lib.hpp"
#include "module_base.hpp"

#include <memory>
#include <string>
#include <utility>

std::pair<dyn_load_lib::lib,
          std::unique_ptr<module_base, module_base::deleter_func_ptr_t>>
dyn_load_module(const char *filename) {
  dyn_load_lib::lib mod_lib{filename, dyn_load_lib::dlopen_flags::LAZY};
  module_base::allocator_func_t *mod_alloc{
      mod_lib.get_symbol<module_base::allocator_func_t>(
          module_base::allocator_func_str)};
  module_base::deleter_func_t *mod_delete{
      mod_lib.get_symbol<module_base::deleter_func_t>(
          module_base::deleter_func_str)};
  std::unique_ptr<module_base, module_base::deleter_func_ptr_t> mod_ptr{
      mod_alloc(), mod_delete};
  return std::make_pair(std::move(mod_lib), std::move(mod_ptr));
}

std::pair<dyn_load_lib::lib,
          std::unique_ptr<module_base, module_base::deleter_func_ptr_t>>
dyn_load_module(const std::string &filename) {
  return dyn_load_module(filename.c_str());
}
