#ifndef THREAD_COMM_HPP
#define THREAD_COMM_HPP

#include <atomic>
#include <compare>
#include <concepts>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace thread_comm {
// TODO reuse buffer

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
  NONE = 0b000,
  READ = 0b001,
  UNREAD = 0b010,
  EXCEPTION = 0b100,
};

} // namespace shared_state_state
template <typename t_value> class shared_state;
template <typename t_value> class shared_state_ptr;
template <typename t_value> class producer;
template <typename t_value> class consumer;

template <typename t_value> class shared_state {
public:
  using t_state_change_callback_func = void (*)(shared_state_state::type);

private:
  std::atomic<shared_state_state::type> m_state;
  std::atomic<t_value *> m_value;
  std::exception_ptr m_exception;
  t_state_change_callback_func m_state_change_callback_func;
  shared_state_state::type m_state_change_subscribed_events;

public:
  shared_state()
      : m_state{shared_state_state::READ}, m_value{nullptr},
        m_exception{nullptr}, m_state_change_callback_func{nullptr},
        m_state_change_subscribed_events{shared_state_state::NONE} {}

  shared_state(t_state_change_callback_func state_change_callback_func,
               shared_state_state::type state_change_subscribed_events)
      : m_state{shared_state_state::READ}, m_value{nullptr},
        m_exception{nullptr},
        m_state_change_callback_func{state_change_callback_func},
        m_state_change_subscribed_events{state_change_subscribed_events} {}

  shared_state(const shared_state &other) = delete;

  shared_state(shared_state &&other) = delete;

  ~shared_state() {
    m_state.store(shared_state_state::READ);
    if (m_value.load()) {
      delete m_value.load();
      m_value.store(nullptr);
    }
  }

  shared_state &operator=(const shared_state &other) = delete;

  shared_state &operator=(shared_state &&other) = delete;

public:
  bool set_value(std::unique_ptr<t_value> value) {
    if (m_state.load() != shared_state_state::EXCEPTION) {
      t_value *old_value{m_value.exchange(value.release())};
      m_state.store(shared_state_state::UNREAD);
      m_state.notify_all();
      if ((m_state_change_subscribed_events & shared_state_state::UNREAD) !=
          0U) {
        m_state_change_callback_func(shared_state_state::UNREAD);
      }
      if (old_value) {
        delete old_value;
        old_value = nullptr;
      }
      return true;
    } else {
      return false;
    }
  }

  bool set_exception(std::exception_ptr exception) {
    if (m_state.load() != shared_state_state::EXCEPTION) {
      m_exception = std::move(exception);
      m_state.store(shared_state_state::EXCEPTION);
      m_state.notify_all();
      if ((m_state_change_subscribed_events & shared_state_state::EXCEPTION) !=
          0U) {
        m_state_change_callback_func(shared_state_state::EXCEPTION);
      }
      return true;
    } else {
      return false;
    }
  }

  std::unique_ptr<t_value> get() {
    switch (m_state.load()) {
    case shared_state_state::READ: {
      wait();
      return get();
    } break;
    case shared_state_state::UNREAD: {
      m_state.store(shared_state_state::READ);
      if ((m_state_change_subscribed_events & shared_state_state::READ) != 0U) {
        m_state_change_callback_func(shared_state_state::READ);
      }
      return std::unique_ptr<t_value>{m_value.exchange(nullptr)};
    } break;
    case shared_state_state::EXCEPTION: {
      std::rethrow_exception(m_exception);
    } break;
    default: {
      return nullptr;
    } break;
    }
  }

  void wait() { m_state.wait(shared_state_state::READ); }
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

  void set_value(std::unique_ptr<t_value> value) {
    if (!valid()) {
      throw error{"no state"};
    }
    m_shared_state_ptr->set_value(std::move(value));
  }

  void set_exception(std::exception_ptr exception) {
    if (!valid()) {
      throw error{"no state"};
    }
    m_shared_state_ptr->set_exception(exception);
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

  std::unique_ptr<t_value> get() {
    if (!valid()) {
      throw error{"no state"};
    }
    return m_shared_state_ptr->get();
  }

  void wait() {
    if (!valid()) {
      throw error{"no state"};
    }

    m_shared_state_ptr->wait();
  }
};

template <typename t_value>
std::pair<producer<t_value>, consumer<t_value>> make_thread_comm_pair(
    typename shared_state<t_value>::t_state_change_callback_func
        state_change_callback_func,
    shared_state_state::type state_change_subscribed_events) {
  shared_state_ptr<t_value> ssp{
      shared_state_ptr<t_value>::make_shared_state_ptr(
          state_change_callback_func, state_change_subscribed_events)};
  return std::make_pair(producer<t_value>{ssp}, consumer<t_value>{ssp});
}

template <typename t_value>
std::pair<producer<t_value>, consumer<t_value>> make_thread_comm_pair() {
  shared_state_ptr<t_value> ssp{
      shared_state_ptr<t_value>::make_shared_state_ptr()};
  return std::make_pair(producer<t_value>{ssp}, consumer<t_value>{ssp});
}

} // namespace thread_comm

#endif
