#ifndef THREAD_COMM_HPP
#define THREAD_COMM_HPP

#include <generic_callback.hpp>

#include <atomic>
#include <compare>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace thread_comm {

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

namespace shared_state_state {
using type = unsigned int;
enum : type {
  null = 0b0000,
  empty = 0b0010,
  value = 0b0100,
  exception = 0b1000,
  all = empty | value | exception,
};

} // namespace shared_state_state

using state_change_callback = generic_callback<shared_state_state::type>;

template <typename t_value> class shared_state;
template <typename t_value> class shared_state_ptr;
template <typename t_value> class producer;
template <typename t_value> class consumer;

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
  shared_state_state::type m_state_change_subscribed_events;

public:
  shared_state()
      : m_value_or_exception{}, m_value_or_exception_mtx{},
        m_state_change_callback{nullptr, nullptr},
        m_state_change_subscribed_events{shared_state_state::null} {}

  shared_state(const state_change_callback &state_change_callback,
               const shared_state_state::type state_change_subscribed_events)
      : m_value_or_exception{}, m_value_or_exception_mtx{},
        m_state_change_callback{state_change_callback},
        m_state_change_subscribed_events{state_change_subscribed_events} {}

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

  t_value get() {
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
      case static_cast<std::size_t>(value_or_exception_idx::value): {
        t_value ret_val{
            std::get<static_cast<std::size_t>(value_or_exception_idx::value)>(
                std::move(m_value_or_exception))};
        m_value_or_exception = std::monostate{};
        lock_m_value_or_exception_mtx.unlock();
        maybe_call_callback(shared_state_state::empty);
        return ret_val;
      } break;
      case static_cast<std::size_t>(value_or_exception_idx::exception): {
        std::rethrow_exception(
            std::get<static_cast<std::size_t>(
                value_or_exception_idx::exception)>(m_value_or_exception));
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
  void maybe_call_callback(const shared_state_state::type state) {
    if ((m_state_change_subscribed_events & state) != 0U) {
      m_state_change_callback.call(state);
    }
  }
};

template <typename t_value> class shared_state_ptr {
private:
  shared_state<t_value> *m_shared_state;
  std::atomic<long> *m_use_count;

public:
  shared_state_ptr() : m_shared_state{nullptr}, m_use_count{nullptr} {}

  shared_state_ptr(std::nullptr_t)
      : m_shared_state{nullptr}, m_use_count{nullptr} {}

  explicit shared_state_ptr(shared_state<t_value> *ssp)
      : m_shared_state{ssp}, m_use_count{new std::atomic<long>{1}} {}

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
      m_use_count = new std::atomic<long>{1};
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

  long use_count() const {
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
  producer() : m_shared_state_ptr{} {}

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
  bool valid() { return static_cast<bool>(m_shared_state_ptr); }

  void swap(producer &other) noexcept {
    using std::swap;
    swap(m_shared_state_ptr, other.m_shared_state_ptr);
  }

  bool put_value(const t_value &value) {
    if (!valid()) {
      throw error{"no state"};
    } else {
      return m_shared_state_ptr->put_value(value);
    }
  }

  bool put_value(t_value &&value) {
    if (!valid()) {
      throw error{"no state"};
    } else {
      return m_shared_state_ptr->put_value(std::move(value));
    }
  }

  bool put_exception(const std::exception_ptr &exception) {
    if (!valid()) {
      throw error{"no state"};
    } else {
      return m_shared_state_ptr->put_exception(exception);
    }
  }

  bool put_exception(std::exception_ptr &&exception) {
    if (!valid()) {
      throw error{"no state"};
    } else {
      return m_shared_state_ptr->put_exception(std::move(exception));
    }
  }
};

template <typename t_value> class consumer {
private:
  shared_state_ptr<t_value> m_shared_state_ptr;

public:
  consumer() : m_shared_state_ptr{} {}

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
  bool valid() { return static_cast<bool>(m_shared_state_ptr); }

  void swap(consumer &other) noexcept {
    using std::swap;
    swap(m_shared_state_ptr, other.m_shared_state_ptr);
  }

  t_value get() {
    if (!valid()) {
      throw error{"no state"};
    } else {
      return m_shared_state_ptr->get();
    }
  }

  void wait() {
    if (!valid()) {
      throw error{"no state"};
    } else {
      m_shared_state_ptr->wait();
    }
  }
};

template <typename t_value>
using t_pair = std::pair<producer<t_value>, consumer<t_value>>;

template <typename t_value>
t_pair<t_value>
make_pair(const state_change_callback &state_change_callback,
          shared_state_state::type state_change_subscribed_events =
              shared_state_state::all) {
  shared_state_ptr<t_value> ssp{
      shared_state_ptr<t_value>::make_shared_state_ptr(
          state_change_callback, state_change_subscribed_events)};
  return std::make_pair(producer<t_value>{ssp}, consumer<t_value>{ssp});
}

template <typename t_value> t_pair<t_value> make_pair() {
  shared_state_ptr<t_value> ssp{
      shared_state_ptr<t_value>::make_shared_state_ptr()};
  return std::make_pair(producer<t_value>{ssp}, consumer<t_value>{ssp});
}
} // namespace thread_comm

#endif
