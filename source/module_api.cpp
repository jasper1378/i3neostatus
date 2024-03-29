#include "module_api.hpp"

#include "i3bar_data.hpp"
#include "i3bar_protocol.hpp"
#include "theme.hpp"
#include "thread_comm.hpp"

#include "libconfigfile/libconfigfile.hpp"

#include <exception>
#include <string>
#include <utility>

const std::string i3neostatus::module_api::config_out::k_valid_name_chars{
    "abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVQXYZ_-"};

i3neostatus::module_api::module_api(
    thread_comm::producer<block> *thread_comm_producer)
    : m_thread_comm_producer{thread_comm_producer} {}

i3neostatus::module_api::module_api(module_api &&other) noexcept
    : m_thread_comm_producer{other.m_thread_comm_producer} {
  other.m_thread_comm_producer = nullptr;
}

i3neostatus::module_api::~module_api() {}

i3neostatus::module_api &
i3neostatus::module_api::operator=(module_api &&other) noexcept {
  if (this != &other) {
    m_thread_comm_producer = other.m_thread_comm_producer;
    other.m_thread_comm_producer = nullptr;
  }
  return *this;
}

void i3neostatus::module_api::put_block(const block &block) {
  m_thread_comm_producer->put_value(block);
}

void i3neostatus::module_api::put_block(block &&block) {
  m_thread_comm_producer->put_value(std::move(block));
}

void i3neostatus::module_api::put_error(const std::exception_ptr &error) {
  m_thread_comm_producer->put_exception(error);
}

void i3neostatus::module_api::put_error(std::exception_ptr &&error) {
  m_thread_comm_producer->put_exception(std::move(error));
}

void i3neostatus::module_api::put_error(const std::exception &error) {
  m_thread_comm_producer->put_exception(error);
}

void i3neostatus::module_api::put_error(std::exception &&error) {
  m_thread_comm_producer->put_exception(std::move(error));
}

void i3neostatus::module_api::set_theme(block &block, const block_state state) {
  block.theme = m_theme->set_block(state);
}
