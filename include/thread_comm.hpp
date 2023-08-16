#ifndef THREAD_COMM_HPP
#define THREAD_COMM_HPP

#include <atomic>
#include <compare>
#include <exception>
#include <iostream>
#include <memory>
#include <variant>

namespace thread_comm {
template <typename t_data> struct shared_state {
  std::atomic<std::variant<t_data, std::exception_ptr> *> m_data;
};

template <typename t_data>
class shared_state_ptr { // TODO optimize dynamic allocations for use_count
private:
  shared_state<t_data> *m_shared_state;
  std::atomic<long> *m_use_count;

public:
  shared_state_ptr() : m_shared_state{nullptr}, m_use_count{nullptr} {}

  shared_state_ptr(std::nullptr_t)
      : m_shared_state{nullptr}, m_use_count{nullptr} {}

  shared_state_ptr(shared_state<t_data> *ssp)
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

  void reset(shared_state<t_data> *ssp) {
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

  shared_state<t_data> *get() const { return m_shared_state; }

  shared_state<t_data> &operator*() const { return *get(); }

  shared_state<t_data> *operator->() const { return get(); }

  long use_count() const {
    if (m_use_count) {
      return m_use_count->load();
    } else {
      return 0;
    }
  }

  explicit operator bool() const { return get() != nullptr; }

public:
  template <typename t_data_1, typename t_data_2>
  friend bool operator==(const shared_state_ptr<t_data_1> &lhs,
                         const shared_state_ptr<t_data_2> &rhs);

  template <typename t_data_1, typename t_data_2>
  friend std::strong_ordering
  operator<=>(const shared_state_ptr<t_data_1> &lhs,
              const shared_state_ptr<t_data_2> &rhs);

  template <typename t_data_1>
  friend bool operator==(const shared_state_ptr<t_data_1> &lhs, std::nullptr_t);

  template <typename t_data_1>
  friend std::strong_ordering operator<=>(const shared_state_ptr<t_data_1> &lhs,
                                          std::nullptr_t);

  template <typename t_data_1>
  friend void swap(shared_state_ptr<t_data_1> &lhs,
                   shared_state_ptr<t_data_1> &rhs);

  template <typename t_data_1>
  friend std::ostream &operator<<(std::ostream &out,
                                  const shared_state_ptr<t_data_1> &ptr);

  template <typename t_data_1, typename... t_args>
  friend shared_state_ptr<t_data> make_shared_state_ptr(t_args &&...args);
};

template <typename t_data_1, typename t_data_2>
bool operator==(const shared_state_ptr<t_data_1> &lhs,
                const shared_state_ptr<t_data_2> &rhs) {
  return (lhs.get() == rhs.get());
}

template <typename t_data_1, typename t_data_2>
std::strong_ordering operator<=>(const shared_state_ptr<t_data_1> &lhs,
                                 const shared_state_ptr<t_data_2> &rhs) {
  return (lhs.get() <=> rhs.get());
}

template <typename t_data_1>
bool operator==(const shared_state_ptr<t_data_1> &lhs, std::nullptr_t) {
  return (lhs.get() == nullptr);
}

template <typename t_data_1>
std::strong_ordering operator<=>(const shared_state_ptr<t_data_1> &lhs,
                                 std::nullptr_t) {
  return (lhs.get() <=> nullptr);
}

template <typename t_data_1>
void swap(shared_state_ptr<t_data_1> &lhs, shared_state_ptr<t_data_1> &rhs) {
  lhs.swap(rhs);
}

template <typename t_data_1>
std::ostream &operator<<(std::ostream &out,
                         const shared_state_ptr<t_data_1> &ptr) {
  out << ptr.get();
  return out;
}

template <typename t_data_1, typename... t_args>
shared_state_ptr<t_data_1> make_shared_state_ptr(t_args &&...args) {
  return shared_state_ptr<t_data_1>{
      new shared_state<t_data_1> *{std::forward<t_args>(args)...}};
}

template <typename t_data> class producer {
private:
  shared_state_ptr<t_data> m_shared_state_ptr;

public:
  producer(const shared_state_ptr<t_data> &ssp) : m_shared_state_ptr{ssp} {}

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

  void put(std::unique_ptr<std::variant<t_data, std::exception_ptr>> data) {
    std::variant<t_data, std::exception_ptr> *old_data{
        m_shared_state_ptr->exchange(data.release())};
    if (old_data) {
      delete old_data;
    }
    // TODO re-use buffer
  }
};

template <typename t_data> class consumer {
private:
  shared_state_ptr<t_data> m_shared_state_ptr;

public:
  consumer(const shared_state_ptr<t_data> &ssp) : m_shared_state_ptr{ssp} {}

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

  std::unique_ptr<std::variant<t_data, std::exception_ptr>> get() {
    std::unique_ptr<std::variant<t_data, std::exception_ptr>> data{
        m_shared_state_ptr->exchange(nullptr)};
    return data;
    // TODO re-use buffer
  }

  // wait() //TODO
};
} // namespace thread_comm

#endif
