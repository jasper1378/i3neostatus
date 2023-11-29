#include "simple_date.hpp"

#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>

simple_date::simple_date() : m_api{}, m_config{} {}

simple_date::~simple_date() {}

module_api::config_out simple_date::init(module_api &&api,
                                         module_api::config_in &&config) {
  m_api = std::move(api);
  m_config = std::move(config);

  module_api::config_out ret_val{"simple_date", false};

  return ret_val;
}

void simple_date::run() {
  auto get_next_whole_second{[]() -> std::chrono::system_clock::time_point {
    return std::chrono::system_clock::time_point{
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()) +
        std::chrono::seconds{1}};
  }};

  std::this_thread::sleep_until(get_next_whole_second());

  while (true) {
    auto now{std::chrono::system_clock::now()};
    std::time_t tnow{std::chrono::system_clock::to_time_t(now)};
    char str[std::size("yyyy-mm-ddThh:mm:ssZ")];
    std::strftime(str, std::size(str), "%FT%TZ", std::localtime(&tnow));

    std::unique_ptr<module_api::block> block{m_api.make_block(module_api::block{
        .full_text = str,
    })};
    m_api.set_block(std::move(block));

    std::this_thread::sleep_until(get_next_whole_second());
  }
}

void simple_date::term() { std::cerr << "simple_date::term()\n"; }

extern "C" {
module_base *allocator() { return new simple_date{}; }

void deleter(module_base *m) { delete m; }
}
