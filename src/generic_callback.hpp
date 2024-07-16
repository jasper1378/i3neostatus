#ifndef I3NEOSTATUS_GENERIC_CALLBACK_HPP
#define I3NEOSTATUS_GENERIC_CALLBACK_HPP

#include <utility>

namespace i3neostatus {

template <typename... T> struct generic_callback {
  void (*func)(void *userdata, T... args);
  void *userdata;

  template <typename... U> void call(U &&...args) {
    func(userdata, std::forward<U>(args)...);
  }
};

} // namespace i3neostatus
#endif
