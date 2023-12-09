#include "dyn_load_lib.hpp"

#include <dlfcn.h>

#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>

dyn_load_lib::error::error(const std::string &what_arg) : base_t{what_arg} {}

dyn_load_lib::error::error(const char *what_arg) : base_t{what_arg} {}

dyn_load_lib::error::error(const error &other) : base_t{other} {}

dyn_load_lib::error::~error() {}

dyn_load_lib::error &dyn_load_lib::error::operator=(const error &other) {
  if (this != &other) {
    base_t::operator=(other);
  }
  return *this;
}

const char *dyn_load_lib::error::what() const noexcept {
  return base_t::what();
}

dyn_load_lib::lib::lib(const char *file_path, dlopen_flags::type flags)
    : m_handle{nullptr} {
  dlerror();
  m_handle = dlopen(file_path, static_cast<int>(flags));
  if (m_handle == nullptr) {
    throw error{std::string{"dlopen(): "} + dlerror()};
  }
}

dyn_load_lib::lib::lib(const std::string &file_path, dlopen_flags::type flags)
    : lib{file_path.c_str(), flags} {}

dyn_load_lib::lib::lib(const std::filesystem::path &file_path,
                       dlopen_flags::type flags)
    : lib{file_path.c_str(), flags} {}

dyn_load_lib::lib::lib(lib &&other) noexcept : m_handle{other.m_handle} {
  other.m_handle = nullptr;
}

dyn_load_lib::lib::~lib() {
  if (m_handle != nullptr) {
    dlclose(m_handle);
  }
}

dyn_load_lib::lib &dyn_load_lib::lib::operator=(lib &&other) {
  if (this != &other) {
    if (m_handle != nullptr) {
      dlerror();
      if (dlclose(m_handle) != 0) {
        throw error{std::string{"dlclose(): "} + dlerror()};
      }
    }
    m_handle = other.m_handle;
    other.m_handle = nullptr;
  }
  return *this;
}

void *dyn_load_lib::lib::get_symbol_impl(void *handle, const char *symbol,
                                         const char *version /*= ""*/) {
  dlerror();
  void *sym_addr{nullptr};
  sym_addr = ((version[0] == '\0') ? (dlsym(handle, symbol))
                                   : (dlvsym(handle, symbol, version)));
  if (sym_addr == nullptr) {
    throw error{std::string{"dlsym(): "} + dlerror()};
  } else {
    return sym_addr;
  }
}

Dl_info dyn_load_lib::lib::get_info_impl(void *addr) {
  dlerror();
  Dl_info ret_val;
  if (dladdr(addr, &ret_val) != 0) {
    throw error{std::string{"dladdr(): "} + dlerror()};
  } else {
    return ret_val;
  }
}
