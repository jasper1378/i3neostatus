#ifndef SIMPLE_DATE_HPP
#define SIMPLE_DATE_HPP

#include "module_dev.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>

using namespace i3neostatus;

class simple_date : public module_base {
private:
  static constexpr std::string k_name{"simple_date"};

private:
  enum class state {
    cont,
    wait,
    stop,
  };

private:
  static constexpr std::string m_k_color_a{"#ffffff"};
  static constexpr std::string m_k_color_b{"#ff0000"};

private:
  module_api *m_api;
  std::string m_format;
  std::atomic<const std::string *> m_color;
  state m_state;
  std::mutex m_state_mtx;
  std::condition_variable m_state_cv;

public:
  simple_date()
      : m_api{}, m_format{}, m_color{&m_k_color_a}, m_state{state::cont},
        m_state_mtx{}, m_state_cv{} {}

  virtual ~simple_date() {}

public:
  virtual module_api::config_out init(module_api *api,
                                      module_api::config_in &&config) override {
    m_api = api;

    if (libconfigfile::node_ptr<libconfigfile::node> np;
        ((config.size() == 1 && config.contains("format")) &&
         ((np = config.at("format"))->get_node_type() ==
          libconfigfile::node_type::String))) {
      m_format = libconfigfile::node_to_base(
          *libconfigfile::node_ptr_cast<libconfigfile::string_node>(np));
    } else {
      throw std::runtime_error{"expected 'format' string in config"};
    }

    return {k_name, true};
  }

  virtual void run() override {
    auto get_next_whole_second{[]() -> std::chrono::system_clock::time_point {
      return std::chrono::system_clock::time_point{
          std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::system_clock::now().time_since_epoch()) +
          std::chrono::seconds{1}};
    }};

    std::size_t buf_size{128};
    char *buf{nullptr};

    auto set_buf_to_size{[&buf_size, &buf](std::size_t new_size) -> void {
      if ((buf = (char *)realloc(buf, (sizeof(char) * new_size))) == nullptr) {
        throw std::runtime_error{"malloc() failled"};
      }

      buf_size = new_size;
    }};

    set_buf_to_size(128);

    while (true) {

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

      module_api::block block{.full_text{buf}, .color{*m_color.load()}};
      m_api->put_block(std::move(block));

      std::unique_lock<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::wait;
      m_state_cv.wait_until(
          lock_m_state_mtx, get_next_whole_second(),
          [this]() -> bool { return m_state != state::wait; });
      if (m_state == state::stop) {
        break;
      } else {
        m_state = state::cont;
      }
    }
    free(buf);
  }

  virtual void term() override {
    {
      std::lock_guard<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::stop;
    }
    m_state_cv.notify_all();
  }

  virtual void on_click_event(module_api::click_event &&click_event) override {
    (void)click_event;
    if (m_color.load() == &m_k_color_a) {
      m_color.store(&m_k_color_b);
    } else {
      m_color.store(&m_k_color_a);
    }
    {
      std::lock_guard<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::cont;
    }
    m_state_cv.notify_all();
  }
};

extern "C" {
module_base *allocator() { return new simple_date{}; }

void deleter(module_base *m) { delete m; }
}

#endif
