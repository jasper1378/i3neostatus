#ifndef I3NEOSTATUS_DYN_LOAD_LIB_HPP
#define I3NEOSTATUS_DYN_LOAD_LIB_HPP

#include "bits-and-bytes/enum_flag_operators.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>

#include <dlfcn.h>

namespace i3neostatus {

namespace dyn_load_lib {
class error : public std::runtime_error {
private:
  using base_t = std::runtime_error;

public:
  explicit error(const std::string &what_arg);
  explicit error(const char *what_arg);
  error(const error &other);

public:
  virtual ~error() override;

public:
  error &operator=(const error &other);

public:
  virtual const char *what() const noexcept override;
};

enum class dlopen_flags : unsigned int {
  lazy = RTLD_LAZY,
  now = RTLD_NOW,
  global = RTLD_GLOBAL,
  local = RTLD_LOCAL,
  nodelete = RTLD_NODELETE,
  noload = RTLD_NOLOAD,
  deepbind = RTLD_DEEPBIND,
};
BITS_AND_BYTES_DEFINE_ENUM_FLAG_OPERATORS_FOR_TYPE(dlopen_flags);

namespace dlsym_pseudohandles {
using type = void *;
static const type DEFAULT{RTLD_DEFAULT};
static const type NEXT{RTLD_NEXT};
} // namespace dlsym_pseudohandles

class lib {
private:
  void *m_handle;

public:
  lib(const char *file_path, dlopen_flags flags);
  lib(const std::string &file_path, dlopen_flags flags);
  lib(const std::filesystem::path &file_path, dlopen_flags flags);
  lib(lib &&other) noexcept;
  lib(const lib &other) = delete;

public:
  ~lib();

public:
  lib &operator=(lib &&other);
  lib &operator=(const lib &other) = delete;

public:
  template <typename T>
  T *get_symbol(const char *symbol, const char *version = "\0") const {
    T *ret_val{nullptr};
    (*reinterpret_cast<void **>(&ret_val)) =
        get_symbol_impl(m_handle, symbol, version);
    return ret_val;
  }

  template <typename T>
  T *get_symbol(const std::string &symbol,
                const std::string &version = "") const {
    return get_symbol<T>(symbol.c_str(), version.c_str());
  }

public:
  template <typename T>
  static T *get_symbol(const dlsym_pseudohandles::type handle,
                       const char *symbol, const char *version = "") {
    T *ret_val{nullptr};
    (*reinterpret_cast<void **>(&ret_val)) =
        get_symbol_impl(handle, symbol, version);
    return ret_val;
  }

  template <typename T>
  static T *get_symbol(const dlsym_pseudohandles::type handle,
                       const std::string &symbol,
                       const std::string &version = "") {
    return get_symbol<T>(handle, symbol.c_str(), version.c_str());
  }

  template <typename T> static Dl_info get_info(T *addr) {
    return get_info_impl(reinterpret_cast<void *>(addr));
  }

private:
  static void *get_symbol_impl(void *handle, const char *symbol,
                               const char *version = "\0");

  static Dl_info get_info_impl(const void *addr);
};
} // namespace dyn_load_lib

} // namespace i3neostatus
#endif
