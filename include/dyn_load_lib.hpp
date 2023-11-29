#ifndef DYN_LOAD_LIB_HPP
#define DYN_LOAD_LIB_HPP

#include <exception>
#include <stdexcept>
#include <string>

#include <dlfcn.h>

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

namespace dlopen_flags {
using type = unsigned int;
enum {
  LAZY = RTLD_LAZY,
  NOW = RTLD_NOW,
  GLOBAL = RTLD_GLOBAL,
  LOCAL = RTLD_LOCAL,
  NODELETE = RTLD_NODELETE,
  NOLOAD = RTLD_NOLOAD,
  DEEPBIND = RTLD_DEEPBIND,
};
} // namespace dlopen_flags

namespace dlsym_pseudohandles {
using type = void *const;
static type DEFAULT{RTLD_DEFAULT};
static type NEXT{RTLD_NEXT};
} // namespace dlsym_pseudohandles

class lib {
private:
  void *m_handle;

public:
  lib(const char *filename, dlopen_flags::type flags);
  lib(const std::string &filename, dlopen_flags::type flags);
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
  static T *get_symbol(dlsym_pseudohandles::type handle, const char *symbol,
                       const char *version = "") {
    T *ret_val{nullptr};
    (*reinterpret_cast<void **>(&ret_val)) =
        get_symbol_impl(handle, symbol, version);
    return ret_val;
  }

  template <typename T>
  static T *get_symbol(dlsym_pseudohandles::type handle,
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

  static Dl_info get_info_impl(void *addr);
};
} // namespace dyn_load_lib

#endif
