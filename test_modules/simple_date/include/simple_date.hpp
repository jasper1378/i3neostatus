#ifndef SIMPLE_DATE_HPP
#define SIMPLE_DATE_HPP

#include "module_api.hpp"
#include "module_base.hpp"

#include <atomic>
#include <string>

class simple_date : public module_base {
private:
  static constexpr std::string k_name{"simple_date"};

  module_api m_api;
  module_api::config_in m_config;
  std::atomic<bool> m_suicide;
  std::string m_format;

public:
  simple_date();

  virtual ~simple_date();

public:
  virtual module_api::config_out init(module_api &&api,
                                      module_api::config_in &&config) override;

  virtual void run() override;

  virtual void term() override;
};

extern "C" {
module_base *allocator();
void deleter(module_base *m);
}

#endif
