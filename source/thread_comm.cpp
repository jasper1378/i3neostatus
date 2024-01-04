#include "thread_comm.hpp"

#include <exception>
#include <stdexcept>
#include <string>

thread_comm::error::error(const std::string &what_arg) : base_t{what_arg} {}

thread_comm::error::error(const char *what_arg) : base_t{what_arg} {}

thread_comm::error::error(const error &other) : base_t{other} {}

thread_comm::error::~error() {}

thread_comm::error &thread_comm::error::operator=(const error &other) {
  if (this != &other) {
    base_t::operator=(other);
  }
  return *this;
}

const char *thread_comm::error::what() const noexcept { return base_t::what(); }
