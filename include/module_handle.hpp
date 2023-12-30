#ifndef MODULE_HANDLE_HPP
#define MODULE_HANDLE_HPP

#include "dyn_load_lib.hpp"
#include "generic_callback.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "module_id.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

class module_handle {
public:
  enum class state_change_type {
    new_block,
    new_exception,
  };

  using state_change_callback = generic_callback<state_change_type>;

private:
  module_id::type m_id;
  std::string m_name;
  std::string m_file_path;
  bool m_click_events_enabled;
  module_api::runtime_settings m_runtime_settings;

  state_change_callback m_state_change_callback;
  dyn_load_lib::lib m_dyn_lib;
  std::unique_ptr<module_base, module_base::deleter_func_ptr_t> m_module;
  thread_comm::producer<module_api::block> m_thread_comm_producer;
  thread_comm::consumer<module_api::block> m_thread_comm_consumer;
  std::thread m_thread;

private:
  static decltype(thread_comm::state_change_callback::func)
      m_s_thread_comm_state_change_callback;

public:
  module_handle(const module_id::type id, std::string &&file_path,
                libconfigfile::map_node &&conf,
                module_api::runtime_settings &&runtime_settings);
                state_change_callback &&state_change_callback);
  module_handle(const module_id::type id, const std::string &file_path,
                const libconfigfile::map_node &conf,
                const module_api::runtime_settings &runtime_settings);
  module_handle(const module_id::type id, std::string &&file_path,
                libconfigfile::map_node &&conf, module_api::runtime_settings &&,
                state_change_callback&& state_change_callback);
  module_handle(const module_id::type id, const std::string &file_path,
                const libconfigfile::map_node &conf,
                const module_api::runtime_settings &,
                const state_change_callback& state_change_callback);
  module_handle(module_handle &&other) noexcept;
  module_handle(const module_handle &other) = delete;

private:
  module_handle(const module_id::type id, std::string &&file_path,
                libconfigfile::map_node &&conf,
                module_api::runtime_settings &&runtime_settings,
                thread_comm::t_pair<module_api::block> &&tc_pair);
  module_handle(const module_id::type id, const std::string &file_path,
                const libconfigfile::map_node &conf,
                const module_api::runtime_settings &runtime_settings,
                thread_comm::t_pair<module_api::block> &&tc_pair);

public:
  ~module_handle();

public:
  module_handle &operator=(module_handle &&other) noexcept;
  module_handle &operator=(const module_handle &other) = delete;

private:
  void do_ctor(libconfigfile::map_node &&conf,
               thread_comm::t_pair<module_api::block> &&tc_pair);

public:
  std::string get_name() const;
  std::string get_file_path() const;
  bool get_click_events_enabled() const;

  module_api::runtime_settings &get_runtime_settings();
  thread_comm::consumer<module_api::block> &get_comm();

  void run();
};

#endif
