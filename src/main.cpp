#include "config_file.hpp"
#include "dyn_load_lib.hpp"
#include "i3bar_protocol.hpp"
#include "misc.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "module_error.hpp"
#include "module_handle.hpp"
#include "module_id.hpp"
#include "thread_comm.hpp"

#include <csignal>

#include <atomic>
#include <iostream>
#include <utility>
#include <vector>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "config file path required\n";
    std::exit(1);
  } else {

    config_file::parsed config{config_file::read(argv[1])};
    if (config.modules.size() > module_id::max) {
      throw std::runtime_error{"too many modules! (max is " +
                               std::to_string(module_id::max) + ")"};
    }
    const module_id::type module_count{config.modules.size()};

    std::vector<module_handle> module_handles{};
    module_handles.reserve(module_count);

    std::atomic<module_id::type> module_last_update{module_id::null};
    struct module_update {
      module_id::type id;
      std::atomic<bool> has_update;
      std::atomic<module_id::type> *last_update;

      module_update(module_id::type id = {}, std::atomic<bool> has_update = {},
                    std::atomic<module_id::type> *last_update = {})
          : id{id}, has_update{has_update.load()}, last_update{last_update} {}

      module_update(const module_update &other)
          : id{other.id}, has_update{other.has_update.load()},
            last_update{other.last_update} {}

      module_update(module_update &&other)
          : id{other.id}, has_update{other.has_update.load()},
            last_update{other.last_update} {}

      ~module_update() = default;

      module_update &operator=(const module_update &other) {
        if (this != &other) {
          id = other.id;
          has_update.store(other.has_update.load());
          last_update = other.last_update;
        }
        return *this;
      }

      module_update &operator=(module_update &&other) {
        if (this != &other) {
          id = other.id;
          has_update.store(other.has_update.load());
          last_update = other.last_update;
        }
        return *this;
      }
    };
    std::vector<module_update> module_updates{};
    module_updates.reserve(module_count);

    const auto module_callback{
        []([[maybe_unused]] thread_comm::shared_state_state::type state,
           void *userdata) -> void {
          module_update *mu{static_cast<module_update *>(userdata)};
          mu->has_update.store(true);
          mu->last_update->store(mu->id);
          mu->last_update->notify_one();
        }};

    for (module_id::type i = 0; i < module_count; ++i) {
      module_updates.emplace_back(i, false, &module_last_update);
      module_handles.emplace_back(
          i, std::move(config.modules[i].file_path),
          std::move(config.modules[i].config), module_api::runtime_settings{},
          thread_comm::state_change_callback{
              module_callback, static_cast<void *>(&module_updates.back())});
      module_handles.back().run();
    }

    i3bar_protocol::print_header({1, SIGSTOP, SIGCONT, true}, std::cout);
    i3bar_protocol::init_statusline(std::cout);
    std::vector<std::string> i3bar_protocol_cache(module_count);

    while (true) {
      module_last_update.wait(module_id::null);
      module_last_update.store(module_id::null);

      for (module_id::type i{0}; i < module_count; ++i) {
        bool expected{true};
        module_updates[i].has_update.compare_exchange_strong(expected, false);
        if (expected) {
          std::unique_ptr<module_api::block> block_content{
              module_handles[i].get_comm().get()};
          std::pair<i3bar_protocol::block, module_id::type> updated_block{
              {i3bar_protocol::block::struct_id{module_handles[i].get_name(),
                                                i},
               *block_content},
              i};
          i3bar_protocol::print_statusline(updated_block, i3bar_protocol_cache,
                                           std::cout);
        }
      }
    }
  }
}
