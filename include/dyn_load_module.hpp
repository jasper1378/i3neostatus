#ifndef DYN_LOAD_MODULE_HPP
#define DYN_LOAD_MODULE_HPP

#include "dyn_load_lib.hpp"
#include "module_base.hpp"

#include <memory>
#include <string>
#include <utility>

std::pair<dyn_load_lib::lib,
          std::unique_ptr<module_base, module_base::deleter_func_ptr_t>>
dyn_load_module(const char *filename);

std::pair<dyn_load_lib::lib,
          std::unique_ptr<module_base, module_base::deleter_func_ptr_t>>
dyn_load_module(const std::string &filename);

#endif
