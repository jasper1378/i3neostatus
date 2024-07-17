#ifndef I3NEOSTATUS_PLUGINS_TEST_PLUGIN_HPP
#define I3NEOSTATUS_PLUGINS_TEST_PLUGIN_HPP

#include "i3neostatus/plugin_dev.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <mutex>
#include <stdexcept>
#include <string>

namespace i3ns = i3neostatus::plugin_dev;

class test_plugin : public i3ns::base {
private:
  static constexpr std::string k_name{"test_plugin"};

private:
  enum class action {
    cont,
    wait,
    stop,
  };

private:
  i3ns::api *m_api;
  std::string m_format;
  std::atomic<i3ns::state> m_state;
  std::atomic<bool> m_hidden;
  action m_action;
  std::mutex m_action_mtx;
  std::condition_variable m_action_cv;

public:
  test_plugin()
      : m_api{}, m_format{}, m_state{i3ns::state::good}, m_hidden{false},
        m_action{action::cont}, m_action_mtx{}, m_action_cv{} {}

  virtual ~test_plugin() {}

public:
  virtual i3ns::config_out init(i3ns::api *api,
                                i3ns::config_in &&config) override {
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

      if (m_hidden.load()) {
        m_api->hide();
      } else {
        m_api->put_block(i3ns::block{{.full_text{buf}}, {m_state.load()}});
      }
      if (m_hidden.load()) {
        std::this_thread::sleep_for(std::chrono::seconds{2});
        m_hidden.store(false);
      }

      std::unique_lock<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::wait;
      m_action_cv.wait_until(
          lock_m_action_mtx, get_next_whole_second(),
          [this]() -> bool { return m_action != action::wait; });
      if (m_action == action::stop) {
        break;
      } else {
        m_action = action::cont;
      }
    }
    free(buf);
  }

  virtual void term() override {
    {
      std::lock_guard<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::stop;
    }
    m_action_cv.notify_all();
  }

  virtual void on_click_event(i3ns::click_event &&click_event) override {
    if ((click_event.modifiers & i3ns::types::click_modifiers::shift) !=
        i3ns::types::click_modifiers::none) {
      m_hidden.store(!m_hidden.load());
    } else {
      if (m_state.load() == i3ns::state::good) {
        m_state.store(i3ns::state::warning);
      } else {
        m_state.store(i3ns::state::good);
      }
    }
    {
      std::lock_guard<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::cont;
    }
    m_action_cv.notify_all();
  }
};

I3NEOSTATUS_PLUGIN_DEV_DEFINE_ALLOC(test_plugin)

#endif
