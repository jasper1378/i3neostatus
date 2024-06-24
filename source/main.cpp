#include "click_event_listener.hpp"
#include "config_file.hpp"
#include "i3bar_data.hpp"
#include "i3bar_protocol.hpp"
#include "message_printing.hpp"
#include "misc.hpp"
#include "module_error.hpp"
#include "module_handle.hpp"
#include "module_id.hpp"

#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>

using namespace i3neostatus;

class update_queue {
public:
  struct update_info {
    module_id::type id;
    class update_queue *update_queue;
    std::atomic<bool> is_buffered;

    update_info(module_id::type id, class update_queue *update_queue = nullptr,
                std::atomic<bool> is_buffered = false)
        : id{id}, update_queue{update_queue}, is_buffered{is_buffered.load()} {}

    update_info(const update_info &other)
        : id{other.id}, update_queue{other.update_queue},
          is_buffered{other.is_buffered.load()} {}

    update_info(update_info &&other)
        : id{other.id}, update_queue{other.update_queue},
          is_buffered{other.is_buffered.load()} {
      other.id = module_id::null;
      other.update_queue = nullptr;
      other.is_buffered.store(false);
    }

    ~update_info() = default;

    update_info &operator=(const update_info &other) {
      if (this != &other) {
        id = other.id;
        update_queue = other.update_queue;
        is_buffered.store(other.is_buffered.load());
      }
      return *this;
    }

    update_info &operator=(update_info &&other) {
      if (this != &other) {
        id = other.id;
        update_queue = other.update_queue;
        is_buffered.store(other.is_buffered.load());

        other.id = module_id::null;
        other.update_queue = nullptr;
        other.is_buffered.store(false);
      }
      return *this;
    }
  };

private:
  std::size_t m_capacity;
  module_id::type *m_buffer;
  std::atomic<std::size_t> m_count;
  std::size_t m_write;
  std::mutex m_write_mtx;
  std::size_t m_read;

public:
  update_queue(std::size_t capacity)
      : m_capacity{capacity}, m_buffer{new module_id::type[m_capacity]{}},
        m_count{0}, m_write{0}, m_read{0} {
    std::fill(m_buffer, m_buffer + m_capacity, module_id::null);
  }

  update_queue(update_queue &&other)
      : m_capacity{other.m_capacity}, m_buffer{other.m_buffer},
        m_count{other.m_count.load()}, m_write{other.m_write},
        m_read{other.m_read} {
    other.m_capacity = 0;
    other.m_buffer = nullptr;
    other.m_count = 0;
    other.m_write = 0;
    other.m_read = 0;
  }

  update_queue(const update_queue &other) = delete;

  ~update_queue() { delete[] m_buffer; }

  update_queue &operator=(update_queue &&other) {
    if (this != &other) {
      delete[] m_buffer;

      m_capacity = other.m_capacity;
      m_buffer = other.m_buffer;
      m_count.store(other.m_count.load());
      m_write = other.m_write;
      m_read = other.m_read;

      other.m_capacity = 0;
      other.m_buffer = nullptr;
      other.m_count.store(0);
      other.m_write = 0;
      other.m_read = 0;
    }
    return *this;
  }

  update_queue &operator=(const update_queue &other) = delete;

public:
  void put(const module_id::type id) {
    {
      std::lock_guard<std::mutex> lock_m_write_mtx{m_write_mtx};
      m_buffer[m_write] = id;
      m_write = inc_and_mod(m_write);
    }
    ++m_count;
    m_count.notify_all();
  }

  module_id::type get() {
    module_id::type id{m_buffer[m_read]};
    m_read = inc_and_mod(m_read);
    --m_count;
    m_count.notify_all();
    return id;
  }

  const std::atomic<std::size_t> &count() const { return m_count; }

private:
  std::size_t inc_and_mod(const std::size_t i) {
    return ((i + 1) % m_capacity);
  }
};


#include "i3bar_data_conversions.hpp"
int main(int argc, char *argv[]) {
  try {
    const char *configuration_file_path{""};

    for (int cur_arg{1}; cur_arg < argc; ++cur_arg) {
      switch (misc::constexpr_hash_string::hash(argv[cur_arg])) {
      case misc::constexpr_hash_string::hash("-c"):
      case misc::constexpr_hash_string::hash("--config"): {
        if (((cur_arg + 1) < argc) && (*argv[cur_arg + 1] != '-')) {
          if (*configuration_file_path == '\0') {
            configuration_file_path = argv[++cur_arg];
          } else {
            message_printing::error((std::string{'"'} + argv[cur_arg] +
                                     "\" option has already been specified\n"),
                                    true);
          }
        } else {
          message_printing::error((std::string{'"'} + argv[cur_arg] +
                                   "\" option requires an argument"),
                                  true);
        }
      } break;
      case misc::constexpr_hash_string::hash("-h"):
      case misc::constexpr_hash_string::hash("--help"): {
        message_printing::help(argv[0]);
        return EXIT_SUCCESS;
      } break;
      case misc::constexpr_hash_string::hash("-v"):
      case misc::constexpr_hash_string::hash("--version"): {
        message_printing::version();
        return EXIT_SUCCESS;
      } break;
      default: {
        message_printing::error(
            (std::string{'"'} + argv[cur_arg] + "\" option is unrecognized\n"),
            true);
      } break;
      }
    }

    config_file::parsed config{
        ((*configuration_file_path == '\0')
             ? (config_file::read())
             : (config_file::read(configuration_file_path)))};
    if (config.modules.size() == 0) {
      message_printing::error("Umm... Are you forgetting something?", true);
    } else if (config.modules.size() > module_id::max) {
      message_printing::error("Fool! That's too many modules!", true);
    }
    const module_id::type module_count{config.modules.size()};

    std::vector<module_handle> module_handles{};
    module_handles.reserve(module_count);

    std::vector<update_queue::update_info> module_updates{};
    module_updates.reserve(module_count);

    update_queue update_queue{module_count};

    const auto module_callback{
        [](void *userdata,
           [[maybe_unused]] module_handle::state_change_type state) -> void {
          update_queue::update_info *module_update{
              static_cast<update_queue::update_info *>(userdata)};
          if (module_update->is_buffered.load() == false) {
            module_update->is_buffered.store(true);
            module_update->update_queue->put(module_update->id);
          }
        }};

    bool any_click_events_enabled{false};
    for (module_id::type i = 0; i < module_count; ++i) {
      module_updates.emplace_back(i, &update_queue, false);
      module_handles.emplace_back(i, std::move(config.modules[i].file_path),
                                  std::move(config.modules[i].config),
                                  module_handle::state_change_callback{
                                      module_callback, &module_updates.back()});
      any_click_events_enabled = any_click_events_enabled ||
                                 module_handles[i].get_click_events_enabled();
      module_handles.back().run();
    }

    click_event_listener click_event_listener{&module_handles, &std::cin};
    if (any_click_events_enabled) {
      click_event_listener.run();
    }

    i3bar_protocol::print_header(
        {1, SIGSTOP, SIGCONT, any_click_events_enabled});
    i3bar_protocol::init_statusline();
    std::vector<std::string> i3bar_cache(module_count);

    while (true) {
      update_queue.count().wait(0);
      for (std::size_t queued_updates{update_queue.count().load()},
           cur_queued_update{};
           cur_queued_update < queued_updates; ++cur_queued_update) {
        module_id::type cur_module_id{update_queue.get()};

        module_updates[cur_module_id].is_buffered.store(false);
        std::variant<module_api::block, std::exception_ptr> block_content_local{
            module_handles[cur_module_id].get_comm().get()};

        auto make_updated_block{
            [&module_handles,
             &cur_module_id](i3bar_data::block::struct_content::struct_local
                                 &&block_content_local)
                -> std::pair<i3bar_data::block, module_id::type> {
              return std::pair<i3bar_data::block, module_id::type>{
                  i3bar_data::block{
                      .id{.name{module_handles[cur_module_id].get_name()},
                          .instance{module_handles[cur_module_id].get_id()}},
                      .content{.global{/**/},
                               .local{std::move(block_content_local)}}},
                  cur_module_id};
            }};

        switch (block_content_local.index()) {
        case 0: {
          i3bar_protocol::print_statusline(
              make_updated_block(std::get<0>(std::move(block_content_local))),
              i3bar_cache, true);
        } break;
        case 1: {
          try {
            std::rethrow_exception(std::get<1>(std::move(block_content_local)));
          } catch (const std::exception &exception) {
            i3bar_protocol::print_statusline(
                make_updated_block(
                    i3bar_data::block::struct_content::struct_local{
                        .full_text{module_error{
                            module_handles[cur_module_id].get_id(),
                            module_handles[cur_module_id].get_name(),
                            module_handles[cur_module_id].get_file_path(),
                            exception.what()}
                                       .what()}}),
                i3bar_cache, true);
          }
        } break;
        }
      }
    }
  } catch (const std::exception &error) {
    print_error(error, true);
  } catch (...) {
    print_error("unknown", true);
  }

  return EXIT_SUCCESS;
}
