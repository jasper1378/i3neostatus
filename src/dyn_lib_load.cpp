#include "dyn_lib_load.hpp"

#include <dlfcn.h>

#include <exception>
#include <stdexcept>
#include <string>

dyn_lib_load::error::error(const std::string &what_arg) : base_t{what_arg} {}

dyn_lib_load::error::error(const char *what_arg) : base_t{what_arg} {}

dyn_lib_load::error::error(const error &other) : base_t{other} {}

dyn_lib_load::error::~error() {}

dyn_lib_load::error &dyn_lib_load::error::operator=(const error &other) {
  if (this != &other) {
    base_t::operator=(other);
  }
  return *this;
}

const char *dyn_lib_load::error::what() const noexcept {
  return base_t::what();
}

dyn_lib_load::lib::lib(const char *filename, dlopen_flags::type flags)
    : m_handle{nullptr} {
  if ((static_cast<bool>(flags & dlopen_flags::LAZY)) ==
      (static_cast<bool>(flags & dlopen_flags::NOW))) {
    throw error{"flags must include exactly one of the constants LAZY or NOW"};
  } else {
    m_handle = dlopen(filename, flags);
    if (!m_handle) {
      throw error{std::string{"dlopen(): "} + dlerror()};
    }
  }
}

dyn_lib_load::lib::lib(lib &&other) noexcept : m_handle{other.m_handle} {
  other.m_handle = nullptr;
}

dyn_lib_load::lib::~lib() {
  if (m_handle) {
    dlclose(m_handle);
  }
}

dyn_lib_load::lib &dyn_lib_load::lib::operator=(lib &&other) {
  if (this != &other) {
    if (m_handle) {
      if (dlclose(m_handle) != 0) {
        throw error{std::string{"dlclose(): "} + dlerror()};
      }
    }
    m_handle = other.m_handle;
    other.m_handle = nullptr;
  }
  return *this;
}

void *dyn_lib_load::lib::get_symbol_impl(void *handle, const char *symbol,
                                         const char *version /*= "\0"*/) {
  void *sym_addr{nullptr};
  sym_addr = ((version[0] == '\0') ? (dlsym(handle, symbol))
                                   : (dlvsym(handle, symbol, version)));
  if (!sym_addr) {
    throw error{std::string{"dlsym(): "} + dlerror()};
  } else {
    return sym_addr;
  }
}

Dl_info dyn_lib_load::lib::get_info_impl(void *addr) {
  Dl_info ret_val;
  if (dladdr(addr, &ret_val) != 0) {
    throw error{std::string{"dladdr(): "} + dlerror()};
  } else {
    return ret_val;
  }
}
