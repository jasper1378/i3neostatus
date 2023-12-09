#ifndef MODULE_HANDLE_HPP
#define MODULE_HANDLE_HPP

#include "dyn_load_lib.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "module_id.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <memory>
#include <string>
#include <thread>

class module_handle {
private:
  module_id::type m_id;
  std::string m_name;
  std::string m_file_path;
  bool m_click_events_enabled;

  dyn_load_lib::lib m_dyn_lib;
  std::unique_ptr<module_base, module_base::deleter_func_ptr_t> m_module;
  thread_comm::producer<module_api::block> m_thread_comm_producer;
  thread_comm::consumer<module_api::block> m_thread_comm_consumer;
  std::thread m_thread;

public:
  module_handle(module_id::type id, std::string &&file_path,
                libconfigfile::map_node &&conf);
  module_handle(module_handle &&other) noexcept;
  module_handle(const module_handle &other) = delete;

public:
  ~module_handle();

public:
  module_handle &operator=(module_handle &&other) noexcept;
  module_handle &operator=(const module_handle &other) = delete;

public:
  std::string get_name() const;
  std::string get_file_path() const;
  bool get_click_events_enabled() const;

  thread_comm::consumer<module_api::block> &get_comm();

  void run();
};

#endif
