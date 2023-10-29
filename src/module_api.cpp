#include "module_api.hpp"

#include "i3bar_protocol.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>

module_api::module_api(thread_comm::producer<block> &&thread_comm_producer)
    : m_thread_comm_producer{std::move(thread_comm_producer)} {}

module_api::module_api(module_api &&other) noexcept
    : m_thread_comm_producer{std::move(other.m_thread_comm_producer)} {}

module_api::~module_api() {}

module_api &module_api::operator=(module_api &&other) noexcept {
  if (this != &other) {
    m_thread_comm_producer = std::move(other.m_thread_comm_producer);
  }
  return *this;
}

void module_api::set_value(std::unique_ptr<block> value) {
  m_thread_comm_producer.set_value(std::move(value));
}

void module_api::set_exception(std::exception_ptr exception) {
  m_thread_comm_producer.set_exception(exception);
}
