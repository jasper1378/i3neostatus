#include "simple_date.hpp"

#include <atomic>
#include <chrono>
#include <ctime>
#include <exception>
#include <stdexcept>
#include <string>
#include <thread>

simple_date::simple_date()
    : m_api{}, m_config{}, m_suicide{false}, m_format{} {}

simple_date::~simple_date() {}

module_api::config_out simple_date::init(module_api &&api,
                                         module_api::config_in &&config) {
  m_api = std::move(api);
  m_config = std::move(config);

  if (libconfigfile::node_ptr<libconfigfile::node> np;
      ((m_config.size() == 1 && m_config.contains("format")) &&
       ((np = m_config.at("format"))->get_node_type() ==
        libconfigfile::node_type::String))) {
    m_format = libconfigfile::node_to_base(
        *libconfigfile::node_ptr_cast<libconfigfile::string_node>(np));
  } else {
    throw std::runtime_error{"expected 'format' string in config"};
  }

  return {k_name, false};
}

void simple_date::run() {
  auto get_next_whole_second{[]() -> std::chrono::system_clock::time_point {
    return std::chrono::system_clock::time_point{
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()) +
        std::chrono::seconds{1}};
  }};

  std::size_t buf_size{128};
  char *buf{nullptr};

  auto set_buf_to_size{[&buf_size, &buf](std::size_t new_size) -> void {
    if (buf != nullptr) {
      free(buf);
    }
    if ((buf = (char *)malloc(sizeof(char) * new_size)) == nullptr) {
      throw std::runtime_error{"malloc() failled"};
    }
    buf_size = new_size;
  }};

  set_buf_to_size(128);

  while (m_suicide.load() == false) {

    auto now{std::chrono::system_clock::now()};
    std::time_t tnow{std::chrono::system_clock::to_time_t(now)};

    while (true) {
      if (std::strftime(buf, buf_size, m_format.c_str(),
                        std::localtime(&tnow)) == 0) {
        set_buf_to_size(buf_size * 2);
      } else {
        break;
      }
    }

    module_api::block block{.full_text{buf}};
    m_api.put_block(std::move(block));

    std::this_thread::sleep_until(get_next_whole_second());
  }
  free(buf);
}

void simple_date::term() { m_suicide.store(true); }

extern "C" {
module_base *allocator() { return new simple_date{}; }

void deleter(module_base *m) { delete m; }
}
