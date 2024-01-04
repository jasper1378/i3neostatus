#ifndef SIMPLE_DATE_HPP
#define SIMPLE_DATE_HPP

#include "module_api.hpp"
#include "module_base.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

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
  module_api m_api;
  module_api::config_in m_config;
  std::string m_format;
  std::atomic<const std::string *> m_color;
  state m_state;
  std::mutex m_state_mtx;
  std::condition_variable m_state_cv;

public:
  simple_date();

  virtual ~simple_date();

public:
  virtual module_api::config_out init(module_api &&api,
                                      module_api::config_in &&config) override;

  virtual void run() override;

  virtual void term() override;

  virtual void on_click_event(module_api::click_event &&click_event) override;
};

extern "C" {
module_base *allocator();
void deleter(module_base *m);
}

#endif
