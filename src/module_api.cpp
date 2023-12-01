#include "module_api.hpp"

#include "i3bar_protocol.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <exception>
#include <memory>
#include <string>
#include <utility>

module_api::module_api() : m_thread_comm_producer{} {}

module_api::module_api(const thread_comm::producer<block> &thread_comm_producer)
    : m_thread_comm_producer{thread_comm_producer} {}

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

void module_api::set_block(std::unique_ptr<block> block) {
  m_thread_comm_producer.set_value(std::move(block));
}

void module_api::set_error(std::exception_ptr error) {
  m_thread_comm_producer.set_exception(std::move(error));
}

void module_api::hide() {
  // TODO
}
