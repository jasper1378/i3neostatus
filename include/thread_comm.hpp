#ifndef I3NEOSTATUS_THREAD_COMM_HPP
#define I3NEOSTATUS_THREAD_COMM_HPP

#include "generic_callback.hpp"
#include "misc.hpp"

#include "bits-and-bytes/enum_flag_operators.hpp"

#include <atomic>
#include <compare>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <iostream>
#include <mutex>
#include <tuple>
#include <utility>
#include <variant>

namespace i3neostatus {

namespace thread_comm {
enum class shared_state_state : unsigned int {
  null = 0b0000,
  empty = 0b0010,
  value = 0b0100,
  exception = 0b1000,
  all = empty | value | exception,
};
BITS_AND_BYTES_DEFINE_ENUM_FLAG_OPERATORS_FOR_TYPE(shared_state_state);

using state_change_callback = generic_callback<shared_state_state>;

template <typename t_value> class shared_state;
template <typename t_value> class shared_state_ptr;
template <typename t_value> class producer;
template <typename t_value> class consumer;

template <template <typename> typename t_intf>
concept concept_interface = misc::same_as_template::same_as<t_intf, producer> ||
                            misc::same_as_template::same_as<t_intf, consumer>;

template <typename t_value> class shared_state {
private:
  std::variant<std::monostate, t_value, std::exception_ptr>
      m_value_or_exception;
  std::mutex m_value_or_exception_mtx;
  std::condition_variable m_value_or_exception_cv;
  enum class value_or_exception_idx : std::size_t {
    empty = 0,
    value = 1,
    exception = 2,
  };

  state_change_callback m_state_change_callback;
  shared_state_state m_state_change_subscribed_events;

public:
  shared_state()
      : m_value_or_exception{}, m_value_or_exception_mtx{},
        m_state_change_callback{nullptr, nullptr},
        m_state_change_subscribed_events{shared_state_state::null} {}

  shared_state(const state_change_callback &state_change_callback,
               const shared_state_state state_change_subscribed_events)
      : m_value_or_exception{}, m_value_or_exception_mtx{},
        m_state_change_callback{state_change_callback},
        m_state_change_subscribed_events{state_change_subscribed_events} {}

public:
  shared_state(const shared_state &other) = delete;

  shared_state(shared_state &&other) = delete;

  ~shared_state() {}

  shared_state &operator=(const shared_state &other) = delete;

  shared_state &operator=(shared_state &&other) = delete;

public:
  bool put_value(const t_value &value) {
    std::unique_lock<std::mutex> lock_m_value_or_exception_mtx{
        m_value_or_exception_mtx};
    if (m_value_or_exception.index() !=
        static_cast<std::size_t>(value_or_exception_idx::exception)) {
      m_value_or_exception = value;
      lock_m_value_or_exception_mtx.unlock();
      m_value_or_exception_cv.notify_one();
      maybe_call_callback(shared_state_state::value);
      return true;
    } else {
      return false;
    }
  }

  bool put_value(t_value &&value) {
    std::unique_lock<std::mutex> lock_m_value_or_exception_mtx{
        m_value_or_exception_mtx};
    if (m_value_or_exception.index() !=
        static_cast<std::size_t>(value_or_exception_idx::exception)) {
      m_value_or_exception = value;
      lock_m_value_or_exception_mtx.unlock();
      m_value_or_exception_cv.notify_one();
      maybe_call_callback(shared_state_state::value);
      return true;
    } else {
      return false;
    }
  }

  bool put_exception(const std::exception_ptr &exception) {
    std::unique_lock<std::mutex> lock_m_value_or_exception_mtx{
        m_value_or_exception_mtx};
    if (m_value_or_exception.index() !=
        static_cast<std::size_t>(value_or_exception_idx::exception)) {
      m_value_or_exception = exception;
      lock_m_value_or_exception_mtx.unlock();
      m_value_or_exception_cv.notify_all();
      maybe_call_callback(shared_state_state::exception);
      return true;
    } else {
      return false;
    }
  }

  bool put_exception(std::exception_ptr &&exception) {
    std::unique_lock<std::mutex> lock_m_value_or_exception_mtx{
        m_value_or_exception_mtx};
    if (m_value_or_exception.index() !=
        static_cast<std::size_t>(value_or_exception_idx::exception)) {
      m_value_or_exception = exception;
      lock_m_value_or_exception_mtx.unlock();
      m_value_or_exception_cv.notify_all();
      maybe_call_callback(shared_state_state::exception);
      return true;
    } else {
      return false;
    }
  }

  bool put_exception(const std::exception &exception) {
    return put_exception(std::make_exception_ptr(exception));
  }

  bool put_exception(std::exception &&exception) {
    return put_exception(std::make_exception_ptr(std::move(exception)));
  }

  std::variant<t_value, std::exception_ptr> get() {
    std::unique_lock<std::mutex> lock_m_value_or_exception_mtx{
        m_value_or_exception_mtx, std::defer_lock_t{}};
    while (true) {
      lock_m_value_or_exception_mtx.lock();
      switch (m_value_or_exception.index()) {
      case static_cast<std::size_t>(value_or_exception_idx::empty): {
        lock_m_value_or_exception_mtx.unlock();
        wait();
        continue;
      } break;
      case static_cast<std::size_t>(value_or_exception_idx::value):
      case static_cast<std::size_t>(value_or_exception_idx::exception): {
        std::variant<t_value, std::exception_ptr> ret_val{
            [this]() -> std::variant<t_value, std::exception_ptr> {
              switch (m_value_or_exception.index()) {
              case static_cast<std::size_t>(value_or_exception_idx::value): {
                return std::get<static_cast<std::size_t>(
                    value_or_exception_idx::value)>(
                    std::move(m_value_or_exception));
              } break;
              case static_cast<std::size_t>(
                  value_or_exception_idx::exception): {
                return std::get<static_cast<std::size_t>(
                    value_or_exception_idx::exception)>(
                    std::move(m_value_or_exception));
              } break;
              default: { // impossible
                return {};
              }
              }
            }()};
        m_value_or_exception = std::monostate{};
        lock_m_value_or_exception_mtx.unlock();
        maybe_call_callback(shared_state_state::empty);
        return ret_val;
      } break;
      }
    }
  }

  void wait() {
    std::unique_lock<std::mutex> lock_m_value_or_exception_mtx{
        m_value_or_exception_mtx};
    m_value_or_exception_cv.wait(
        lock_m_value_or_exception_mtx, [this]() -> bool {
          return m_value_or_exception.index() !=
                 static_cast<std::size_t>(value_or_exception_idx::empty);
        });
  }

private:
  void maybe_call_callback(const shared_state_state state) {
    if (static_cast<std::underlying_type_t<shared_state_state>>(
            m_state_change_subscribed_events & state) != 0U) {
      m_state_change_callback.call(state);
    }
  }
};

template <typename t_value> class shared_state_ptr {
private:
  shared_state<t_value> *m_shared_state;
  std::atomic<std::size_t> *m_use_count;

public:
  shared_state_ptr() : m_shared_state{nullptr}, m_use_count{nullptr} {}

  shared_state_ptr(std::nullptr_t)
      : m_shared_state{nullptr}, m_use_count{nullptr} {}

  explicit shared_state_ptr(shared_state<t_value> *ssp)
      : m_shared_state{ssp}, m_use_count{new std::atomic<std::size_t>{1}} {}

  shared_state_ptr(const shared_state_ptr &other)
      : m_shared_state{other.m_shared_state}, m_use_count{other.m_use_count} {
    if (m_use_count != nullptr) {
      ++(*m_use_count);
    }
  }

  shared_state_ptr(shared_state_ptr &&other) noexcept
      : m_shared_state{other.m_shared_state}, m_use_count{other.m_use_count} {
    other.m_shared_state = nullptr;
    other.m_use_count = nullptr;
  }

  ~shared_state_ptr() { reset(nullptr); }

  shared_state_ptr &operator=(const shared_state_ptr &other) {
    if (this != &other) {
      reset(nullptr);
      m_shared_state = other.m_shared_state;
      m_use_count = other.m_use_count;
      if (m_use_count != nullptr) {
        ++(*m_use_count);
      }
    }
    return *this;
  }

  shared_state_ptr &operator=(shared_state_ptr &&other) noexcept {
    if (this != &other) {
      reset(nullptr);
      m_shared_state = other.m_shared_state;
      other.m_shared_state = nullptr;
      m_use_count = other.m_use_count;
      other.m_use_count = nullptr;
    }
    return *this;
  }

public:
  void reset() { reset(nullptr); }

  void reset(shared_state<t_value> *ssp) {
    if (m_use_count != nullptr) {
      --(*m_use_count);
      if (*m_use_count == 0) {
        delete m_use_count;
        m_use_count = nullptr;
        delete m_shared_state;
        m_shared_state = nullptr;
      }
    }
    if (ssp) {
      m_shared_state = ssp;
      m_use_count = new std::atomic<std::size_t>{1};
    }
  }

  void swap(shared_state_ptr &other) noexcept {
    using std::swap;
    swap(m_shared_state, other.m_shared_state);
    swap(m_use_count, other.m_use_count);
  }

  shared_state<t_value> *get() const { return m_shared_state; }

  shared_state<t_value> &operator*() const { return *get(); }

  shared_state<t_value> *operator->() const { return get(); }

  std::size_t use_count() const {
    if (m_use_count != nullptr) {
      return m_use_count->load();
    } else {
      return 0;
    }
  }

  explicit operator bool() const { return get() != nullptr; }

public:
  template <typename... t_args>
  static shared_state_ptr<t_value> make_shared_state_ptr(t_args &&...args) {
    return shared_state_ptr<t_value>{
        new shared_state<t_value>{std::forward<t_args>(args)...}};
  }

public:
  template <typename t_value_1, typename t_value_2>
  friend bool operator==(const shared_state_ptr<t_value_1> &lhs,
                         const shared_state_ptr<t_value_2> &rhs);

  template <typename t_value_1, typename t_value_2>
  friend std::strong_ordering
  operator<=>(const shared_state_ptr<t_value_1> &lhs,
              const shared_state_ptr<t_value_2> &rhs);

  template <typename t_value_1>
  friend bool operator==(const shared_state_ptr<t_value_1> &lhs,
                         std::nullptr_t);

  template <typename t_value_1>
  friend std::strong_ordering
  operator<=>(const shared_state_ptr<t_value_1> &lhs, std::nullptr_t);

  template <typename t_value_1>
  friend void swap(shared_state_ptr<t_value_1> &lhs,
                   shared_state_ptr<t_value_1> &rhs);

  template <typename t_value_1>
  friend std::ostream &operator<<(std::ostream &out,
                                  const shared_state_ptr<t_value_1> &ptr);
};

template <typename t_value_1, typename t_value_2>
bool operator==(const shared_state_ptr<t_value_1> &lhs,
                const shared_state_ptr<t_value_2> &rhs) {
  return (lhs.get() == rhs.get());
}

template <typename t_value_1, typename t_value_2>
std::strong_ordering operator<=>(const shared_state_ptr<t_value_1> &lhs,
                                 const shared_state_ptr<t_value_2> &rhs) {
  return (lhs.get() <=> rhs.get());
}

template <typename t_value_1>
bool operator==(const shared_state_ptr<t_value_1> &lhs, std::nullptr_t) {
  return (lhs.get() == nullptr);
}

template <typename t_value_1>
std::strong_ordering operator<=>(const shared_state_ptr<t_value_1> &lhs,
                                 std::nullptr_t) {
  return (lhs.get() <=> nullptr);
}

template <typename t_value_1>
void swap(shared_state_ptr<t_value_1> &lhs, shared_state_ptr<t_value_1> &rhs) {
  lhs.swap(rhs);
}

template <typename t_value_1>
std::ostream &operator<<(std::ostream &out,
                         const shared_state_ptr<t_value_1> &ptr) {
  out << ptr.get();
  return out;
}

template <typename t_value> class producer {
private:
  shared_state_ptr<t_value> m_shared_state_ptr;

public:
  explicit producer(const shared_state_ptr<t_value> &ssp)
      : m_shared_state_ptr{ssp} {}

  producer(const producer &other)
      : m_shared_state_ptr{other.m_shared_state_ptr} {}

  producer(producer &&other) noexcept
      : m_shared_state_ptr{std::move(other.m_shared_state_ptr)} {}

  ~producer(){};

  producer &operator=(const producer &other) {
    if (this != &other) {
      m_shared_state_ptr = other.m_shared_state_ptr;
    }
    return *this;
  }

  producer &operator=(producer &&other) noexcept {
    if (this != &other) {
      m_shared_state_ptr = std::move(other.m_shared_state_ptr);
    }
    return *this;
  }

public:
  void swap(producer &other) noexcept {
    using std::swap;
    swap(m_shared_state_ptr, other.m_shared_state_ptr);
  }

  bool put_value(const t_value &value) {
    return m_shared_state_ptr->put_value(value);
  }

  bool put_value(t_value &&value) {
    return m_shared_state_ptr->put_value(std::move(value));
  }

  bool put_exception(const std::exception_ptr &exception) {
    return m_shared_state_ptr->put_exception(exception);
  }

  bool put_exception(std::exception_ptr &&exception) {
    return m_shared_state_ptr->put_exception(std::move(exception));
  }

  bool put_exception(const std::exception &exception) {
    return m_shared_state_ptr->put_exception(std::move(exception));
  }

  bool put_exception(std::exception &&exception) {
    return m_shared_state_ptr->put_exception(std::move(exception));
  }

  const shared_state_ptr<t_value> &get_underlying() const {
    return m_shared_state_ptr;
  }

  shared_state_ptr<t_value> get_underlying() { return m_shared_state_ptr; }
};

template <typename t_value> class consumer {
private:
  shared_state_ptr<t_value> m_shared_state_ptr;

public:
  explicit consumer(const shared_state_ptr<t_value> &ssp)
      : m_shared_state_ptr{ssp} {}

  consumer(const consumer &other)
      : m_shared_state_ptr{other.m_shared_state_ptr} {}

  consumer(consumer &&other) noexcept
      : m_shared_state_ptr{std::move(other.m_shared_state_ptr)} {}

  ~consumer(){};

  consumer &operator=(const consumer &other) {
    if (this != &other) {
      m_shared_state_ptr = other.m_shared_state_ptr;
    }
    return *this;
  }

  consumer &operator=(consumer &&other) noexcept {
    if (this != &other) {
      m_shared_state_ptr = std::move(other.m_shared_state_ptr);
    }
    return *this;
  }

public:
  void swap(consumer &other) noexcept {
    using std::swap;
    swap(m_shared_state_ptr, other.m_shared_state_ptr);
  }

  std::variant<t_value, std::exception_ptr> get() {
    return m_shared_state_ptr->get();
  }

  void wait() { m_shared_state_ptr->wait(); }

  const shared_state_ptr<t_value> &get_underlying() const {
    return m_shared_state_ptr;
  }

  shared_state_ptr<t_value> get_underlying() { return m_shared_state_ptr; }
};

template <typename t_value, template <typename> typename t_intf>
  requires concept_interface<t_intf>
t_intf<t_value> make() {
  return t_intf<t_value>{shared_state_ptr<t_value>::make_shared_state_ptr()};
}

template <typename t_value, template <typename> typename t_intf>
  requires concept_interface<t_intf>
t_intf<t_value> make(const state_change_callback &state_change_callback,
                     const shared_state_state state_change_subscribed_events =
                         shared_state_state::all) {
  return t_intf<t_value>{shared_state_ptr<t_value>::make_shared_state_ptr(
      state_change_callback, state_change_subscribed_events)};
}

template <template <typename> typename t_intf_1, typename t_value,
          template <typename> typename t_intf_2>
  requires concept_interface<t_intf_1> && concept_interface<t_intf_2>
t_intf_1<t_value> make_from(const t_intf_2<t_value> &intf_2) {
  return t_intf_1<t_value>{intf_2.get_underlying()};
}

template <typename t_value, template <typename> typename... t_intf>
std::tuple<t_intf<t_value>...> make_set() {
  shared_state_ptr<t_value> ssp{
      shared_state_ptr<t_value>::make_shared_state_ptr()};
  return std::tuple{t_intf<t_value>{ssp}...};
}

template <typename t_value, template <typename> typename... t_intf>
std::tuple<t_intf<t_value>...>
make_set(const state_change_callback &state_change_callback,
         const shared_state_state state_change_subscribed_events =
             shared_state_state::all) {
  shared_state_ptr<t_value> ssp{
      shared_state_ptr<t_value>::make_shared_state_ptr(
          state_change_callback, state_change_subscribed_events)};
  return std::tuple{t_intf<t_value>{ssp}...};
}

} // namespace thread_comm

} // namespace i3neostatus
#endif
