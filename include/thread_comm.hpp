#ifndef THREAD_COMM_HPP
#define THREAD_COMM_HPP

#include <atomic>
#include <compare>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <utility>
#include <variant>

namespace thread_comm {
// TODO reuse buffer

template <typename t_value> class shared_state {
private:
  enum class DATA_IDX : std::size_t {
    VALUE = 0,
    EXCEPTION = 1,
  };

private:
  std::atomic<bool> m_updated;
  std::atomic<std::variant<t_value *, std::exception_ptr> *> m_data;

public:
  shared_state() : m_updated{false}, m_data{nullptr} {}

  shared_state(shared_state &&other) noexcept
      : m_updated{other.m_updated.exchange(false)},
        m_data{other.m_data.exchange(nullptr)} {}

  shared_state(const shared_state &other) = delete;

  ~shared_state() { destruct_data(m_data.exchange(nullptr)); }

  shared_state &operator=(const shared_state &other) = delete;

  shared_state &operator=(shared_state &&other) noexcept {
    if (this != &other) {
      std::variant<t_value *, std::exception_ptr> *old_data{
          m_data.exchange(other.m_data.exchange(nullptr))};
      m_updated.store(other.m_updated.exchange(false));
      destruct_data(old_data);
    }
    return *this;
  }

public:
  void set_value(std::unique_ptr<t_value> value) {
    std::variant<t_value *, std::exception_ptr> *new_data{
        new std::variant<t_value *, std::exception_ptr>{value.release()}};
    std::variant<t_value *, std::exception_ptr> *old_data{
        m_data.exchange(new_data)};
    m_updated.store(true);
    m_updated.notify_all();
    destruct_data(old_data);
  }

  void set_exception(std::exception_ptr exception) {
    std::variant<t_value *, std::exception_ptr> *new_data{
        new std::variant<t_value *, std::exception_ptr>{exception}};
    std::variant<t_value *, std::exception_ptr> *old_data{
        m_data.exchange(new_data)};
    m_updated.store(true);
    m_updated.notify_all();
    destruct_data(old_data);
  }

  std::unique_ptr<t_value> get() {
    // TODO wait()?
    std::variant<t_value *, std::exception_ptr> *data{m_data.exchange(nullptr)};
    m_updated.store(false);
    if (data) {
      switch (data->index()) {
      case static_cast<std::size_t>(DATA_IDX::VALUE): {
        t_value *value{nullptr};
        std::swap(value,
                  std::get<static_cast<std::size_t>(DATA_IDX::VALUE)>(*data));
        destruct_data(data);
        return std::unique_ptr<t_value>(value);
      } break;

      case static_cast<std::size_t>(DATA_IDX::EXCEPTION): {
        std::exception_ptr exception{
            std::get<static_cast<std::size_t>(DATA_IDX::EXCEPTION)>(*data)};
        destruct_data(data);
        std::rethrow_exception(exception);
      } break;

      default: {
        return nullptr;
      } break;
      }
    } else {
      return nullptr;
    }
  }

  void wait() { m_updated.wait(false); }

private:
  void destruct_data(std::variant<t_value *, std::exception_ptr> *old_data) {
    if (old_data) {
      if (old_data->index() == static_cast<std::size_t>(DATA_IDX::VALUE)) {
        t_value *old_value{
            std::get<static_cast<std::size_t>(DATA_IDX::VALUE)>(*old_data)};
        if (old_value) {
          delete old_value;
          old_value = nullptr;
        }
      }
      delete old_data;
      old_data = nullptr;
    }
  }
};

template <typename t_value>
class shared_state_ptr { // TODO optimize dynamic allocations for use_count
private:
  shared_state<t_value> *m_shared_state;
  std::atomic<long> *m_use_count;

public:
  shared_state_ptr() : m_shared_state{nullptr}, m_use_count{nullptr} {}

  shared_state_ptr(std::nullptr_t)
      : m_shared_state{nullptr}, m_use_count{nullptr} {}

  shared_state_ptr(shared_state<t_value> *ssp)
      : m_shared_state{ssp}, m_use_count{new std::atomic<long>{1}} {}

  shared_state_ptr(const shared_state_ptr &other)
      : m_shared_state{other.m_shared_state}, m_use_count{other.m_use_count} {
    if (m_use_count) {
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
      if (m_use_count) {
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
    if (m_use_count) {
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
    if (m_use_count) {
      return m_use_count->load();
    } else {
      return 0;
    }
  }

  explicit operator bool() const { return get() != nullptr; }

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

  template <typename t_value_1, typename... t_args>
  friend shared_state_ptr<t_value> make_shared_state_ptr(t_args &&...args);
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

template <typename t_value_1, typename... t_args>
shared_state_ptr<t_value_1> make_shared_state_ptr(t_args &&...args) {
  return shared_state_ptr<t_value_1>{
      new shared_state<t_value_1>{std::forward<t_args>(args)...}};
}

template <typename t_value> class producer {
private:
  shared_state_ptr<t_value> m_shared_state_ptr;

public:
  producer(const shared_state_ptr<t_value> &ssp) : m_shared_state_ptr{ssp} {}

  producer(producer &&other) noexcept
      : m_shared_state_ptr{std::move(other.m_shared_state_ptr)} {}

  producer(const producer &other) = delete;

  ~producer(){};

  producer &operator=(producer &&other) noexcept {
    m_shared_state_ptr = std::move(other.m_shared_state_ptr);
    return *this;
  }

  producer &operator=(const producer &other) = delete;

public:
  void swap(producer &other) noexcept {
    using std::swap;
    swap(m_shared_state_ptr, other.m_shared_state_ptr);
  }

  void set_value(std::unique_ptr<t_value> value) {
    m_shared_state_ptr->set_value(std::move(value));
  }

  void set_exception(std::exception_ptr exception) {
    m_shared_state_ptr->set_exception(exception);
  }
};

template <typename t_value> class consumer {
private:
  shared_state_ptr<t_value> m_shared_state_ptr;

public:
  consumer(const shared_state_ptr<t_value> &ssp) : m_shared_state_ptr{ssp} {}

  consumer(consumer &&other) noexcept
      : m_shared_state_ptr{std::move(other.m_shared_state_ptr)} {}

  consumer(const consumer &other) = delete;

  ~consumer(){};

  consumer &operator=(consumer &&other) noexcept {
    m_shared_state_ptr = std::move(other.m_shared_state_ptr);
    return *this;
  }

  consumer &operator=(const consumer &other) = delete;

public:
  void swap(consumer &other) noexcept {
    using std::swap;
    swap(m_shared_state_ptr, other.make_shared_state_ptr);
  }

  std::unique_ptr<t_value> get() { return m_shared_state_ptr->get(); }

  void wait() { m_shared_state_ptr->wait(); }
};

template <typename t_value>
std::pair<producer<t_value>, consumer<t_value>> make_thread_comm_pair() {
  shared_state_ptr ssp{make_shared_state_ptr<t_value>()};
  return std::make_pair<producer<t_value>, consumer<t_value>>(ssp, ssp);
}
} // namespace thread_comm

#endif
