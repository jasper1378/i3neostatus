#ifndef DYN_LIB_LOAD_HPP
#define DYN_LIB_LOAD_HPP

#include <exception>
#include <stdexcept>
#include <string>

#include <dlfcn.h>

namespace dyn_lib_load {
class error : public std::runtime_error {
private:
  using base_t = std::runtime_error;

public:
  error(const std::string &what_arg);
  error(const char *what_arg);
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
using type = void *;
static const type DEFAULT{RTLD_DEFAULT};
static const type NEXT{RTLD_NEXT};
} // namespace dlsym_pseudohandles

class lib {
private:
  void *m_handle;

public:
  lib(const char *filename, dlopen_flags::type flags);
  lib(lib &&other) noexcept;
  lib(const lib &other) = delete;

public:
  ~lib();

public:
  lib &operator=(lib &&other);
  lib &operator=(const lib &other) = delete;

public:
  template <typename R, typename... Args>
  auto get_symbol(const char *symbol, const char *version = "\0") const
      -> R (*)(Args...) {
    R (*ret_val)(Args...);
    (*reinterpret_cast<void **>(&ret_val)) =
        get_symbol_impl(m_handle, symbol, version);
    return ret_val;
  }

  template <typename T>
  auto get_symbol(const char *symbol, const char *version = "\0") const -> T * {
    return (reinterpret_cast<T *>(get_symbol_impl(m_handle, symbol, version)));
  }

public:
  template <typename R, typename... Args>
  static auto get_symbol(dlsym_pseudohandles::type handle, const char *symbol,
                         const char *version = "\0") -> R (*)(Args...) {
    R (*ret_val)(Args...);
    (*reinterpret_cast<void **>(&ret_val)) =
        get_symbol_impl(handle, symbol, version);
    return ret_val;
  }

  template <typename T>
  static auto get_symbol(dlsym_pseudohandles::type handle, const char *symbol,
                         const char *version = "\0") -> T * {
    return (reinterpret_cast<T *>(get_symbol_impl(handle, symbol, version)));
  }

  template <typename R, typename... Args>
  static Dl_info get_info(R (*addr)(Args...)) {
    return get_info_impl(reinterpret_cast<void *>(addr));
  }

  template <typename T> static Dl_info get_info(T *addr) {
    return get_info_impl(reinterpret_cast<void *>(addr));
  }

private:
  static void *get_symbol_impl(void *handle, const char *symbol,
                               const char *version = "\0");

  static Dl_info get_info_impl(void *addr);
};
} // namespace dyn_lib_load

#endif
