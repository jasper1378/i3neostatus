#ifndef GENERIC_CALLBACK_HPP
#define GENERIC_CALLBACK_HPP

#include <utility>

template <typename... T> struct generic_callback {
  void (*func)(void *userdata, T... args);
  void *userdata;

  void call(T &&...args) { func(userdata, std::forward<T>(args)...); }
};

#endif
