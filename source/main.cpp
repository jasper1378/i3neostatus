#include "block_state.hpp"
#include "click_event_listener.hpp"
#include "config_file.hpp"
#include "hide_block.hpp"
#include "i3bar_data.hpp"
#include "i3bar_protocol.hpp"
#include "make_block.hpp"
#include "message_printing.hpp"
#include "misc.hpp"
#include "module_api.hpp"
#include "module_error.hpp"
#include "module_handle.hpp"
#include "module_id.hpp"

#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <tuple>
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
    bool click_events_enabled{false};

    std::vector<module_handle> module_handles{};
    module_handles.reserve(module_count);
    std::vector<update_queue::update_info> module_updates{};
    module_updates.reserve(module_count);
    update_queue update_queue{module_count};

    std::pair<std::vector<struct i3bar_data::block>, std::vector<block_state>>
        content_cache{std::piecewise_construct, std::forward_as_tuple(),
                      std::forward_as_tuple(module_count, block_state::idle)};
    content_cache.first.reserve(module_count);
    std::vector<module_id::type> module_id_to_active_index(module_count,
                                                           module_id::null);
    std::vector<module_id::type> active_index_to_module_id(module_count,
                                                           module_id::null);

    std::vector<std::string> content_string_cache(module_count);
    std::vector<std::string> separator_string_cache(
        (config.general.custom_separators) ? (module_count + 1) : (0));

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

    for (module_id::type cur_module_id{0}; cur_module_id < module_count;
         ++cur_module_id) {
      module_updates.emplace_back(cur_module_id, &update_queue, false);
      module_handles.emplace_back(
          cur_module_id, std::move(config.modules[cur_module_id].file_path),
          std::move(config.modules[cur_module_id].config),
          module_handle::state_change_callback{module_callback,
                                               &module_updates.back()});
      click_events_enabled =
          (click_events_enabled ||
           module_handles[cur_module_id].get_click_events_enabled());
      module_handles.back().run();
      content_cache.first.emplace_back(i3bar_data::block{
          .id{.name{module_handles.back().get_name()},
              .instance{cur_module_id}},
          .data{.module{
              hide_block::set<struct i3bar_data::block::data::module>()}}});
    }

    click_event_listener click_event_listener{&module_handles, &std::cin};
    if (click_events_enabled) {
      click_event_listener.run();
    }

    i3bar_protocol::print_header({1, SIGSTOP, SIGCONT, click_events_enabled});
    i3bar_protocol::init_statusline();

    while (true) {
      update_queue.count().wait(0);
      for (std::size_t queued_updates{update_queue.count().load()},
           cur_queued_update{};
           cur_queued_update < queued_updates; ++cur_queued_update) {
        const module_id::type cur_module_id{update_queue.get()};
        module_updates[cur_module_id].is_buffered.store(false);

        bool hide_previous{hide_block::get(content_cache.first[cur_module_id])};
        std::variant<module_api::block, std::exception_ptr> content_module{
            module_handles[cur_module_id].get_comm().get()};

        switch (content_module.index()) {
        case 0: {
          std::tie(content_cache.first[cur_module_id].data.module,
                   content_cache.second[cur_module_id]) =
              std::get<0>(std::move(content_module));
        } break;
        case 1: {
          try {
            std::rethrow_exception(std::get<1>(std::move(content_module)));
          } catch (const std::exception &exception) {
            content_cache.first[cur_module_id].data.module = {
                .full_text{module_error{
                    cur_module_id, module_handles[cur_module_id].get_name(),
                    module_handles[cur_module_id].get_file_path(),
                    exception.what()}
                               .what()},
                .short_text{std::nullopt},
                .min_width{std::nullopt},
                .align{std::nullopt},
                .urgent{true},
                .markup{i3bar_data::types::markup::none}};
            content_cache.second[cur_module_id] = block_state::error;
          }
        } break;
        default: {
          throw std::runtime_error{"impossible!"};
        } break;
        }

        bool hide_current{hide_block::get(content_cache.first[cur_module_id])};

        const auto make_separator_left{
            [&config, module_count, &content_cache, &module_id_to_active_index,
             &active_index_to_module_id](const module_id::type cur_module_id)
                -> std::pair<i3bar_data::block, module_id::type> {
              if (cur_module_id == module_id::null) {
                return {i3bar_data::block{.data{.module{hide_block::set<
                            struct i3bar_data::block::data::module>()}}},
                        module_id::null};
              }
              module_id::type left_module_id{
                  ((module_id_to_active_index[cur_module_id] != 0)
                       ? (active_index_to_module_id
                              [module_id_to_active_index[cur_module_id] - 1])
                       : (module_id::null))};
              return {
                  make_block::separator(
                      config.theme,
                      ((left_module_id != module_id::null)
                           ? (&content_cache.first[left_module_id]
                                   .data.program.theme)
                           : (nullptr)),
                      &content_cache.first[cur_module_id].data.program.theme),
                  cur_module_id};
            }};
        const auto make_separator_right{
            [&config, module_count, &content_cache, &module_id_to_active_index,
             &active_index_to_module_id](const module_id::type cur_module_id)
                -> std::pair<i3bar_data::block, module_id::type> {
              if (cur_module_id == module_id::null) {
                return {i3bar_data::block{.data{.module{hide_block::set<
                            struct i3bar_data::block::data::module>()}}},
                        module_id::null};
              }
              module_id::type right_module_id{
                  ((((module_id_to_active_index[cur_module_id] + 1 <
                      module_count)) &&
                    (active_index_to_module_id
                         [module_id_to_active_index[cur_module_id] + 1] !=
                     module_id::null))
                       ? (active_index_to_module_id
                              [module_id_to_active_index[cur_module_id] + 1])
                       : (module_id::null))};
              return {
                  make_block::separator(
                      config.theme,
                      &content_cache.first[cur_module_id].data.program.theme,
                      ((right_module_id != module_id::null)
                           ? (&content_cache.first[right_module_id]
                                   .data.program.theme)
                           : (nullptr))),
                  ((right_module_id != module_id::null) ? (right_module_id)
                                                        : (module_count))};
            }};
        const auto make_separators{
            [&make_separator_left,
             &make_separator_right](const module_id::type cur_module_id)
                -> std::pair<decltype(make_separator_left(cur_module_id)),
                             decltype(make_separator_right(cur_module_id))> {
              return {make_separator_left(cur_module_id),
                      make_separator_right(cur_module_id)};
            }};

        if (hide_previous != hide_current) {
          if (hide_current) {
            for (module_id::type i{module_id_to_active_index[cur_module_id]};
                 i < (module_count - 1); ++i) {
              active_index_to_module_id[i] = active_index_to_module_id[i + 1];
            }
            active_index_to_module_id.back() = module_id::null;
            for (module_id::type i{cur_module_id + 1}; i < module_count; ++i) {
              if (module_id_to_active_index[i] != module_id::null) {
                --module_id_to_active_index[i];
              }
            }
            module_id_to_active_index[cur_module_id] = module_id::null;
          } else {
            for (module_id::type i{0};; ++i) {
              if ((active_index_to_module_id[i] > cur_module_id) ||
                  (active_index_to_module_id[i] == module_id::null)) {
                for (module_id::type j{module_count}; (j--) > (i + 1);) {
                  active_index_to_module_id[j] =
                      active_index_to_module_id[j - 1];
                }
                active_index_to_module_id[i] = cur_module_id;
                for (module_id::type j{cur_module_id + 1}; j < module_count;
                     ++j) {
                  if (module_id_to_active_index[j] != module_id::null) {
                    ++module_id_to_active_index[j];
                  }
                }
                module_id_to_active_index[cur_module_id] = i;
                break;
              }
            }
          }

          for (module_id::type i{0}; i < module_count; ++i) {
            if (!hide_block::get(content_cache.first[i])) {
              content_cache.first[i].data.program = make_block::content(
                  config.theme, content_cache.second[i],
                  (((module_id_to_active_index[i]) % 2) != 0),
                  config.general.custom_separators);
            }
          }

          if (config.general.custom_separators) {
            i3bar_protocol::print_statusline(
                content_cache.first, content_string_cache,
                [module_count, &module_id_to_active_index,
                 &active_index_to_module_id, &make_separator_left,
                 &make_separator_right]() -> std::vector<i3bar_data::block> {
                  if (active_index_to_module_id.front() == module_id::null) {
                    return std::vector<i3bar_data::block>{
                        (module_count + 1),
                        i3bar_data::block{.data{.module{hide_block::set<
                            struct i3bar_data::block::data::module>()}}}};
                  } else {
                    std::vector<i3bar_data::block> ret_val;
                    ret_val.reserve(module_count + 1);
                    module_id::type last_module_id{module_id::null};
                    for (module_id::type i{0}; i < module_count; ++i) {
                      if ((active_index_to_module_id[i] == module_id::null) &&
                          (last_module_id == module_id::null)) {
                        last_module_id = active_index_to_module_id[i - 1];
                      }
                      ret_val.emplace_back(
                          make_separator_left(
                              (module_id_to_active_index[i] != module_id::null)
                                  ? (i)
                                  : (module_id::null))
                              .first);
                    }
                    last_module_id = ((last_module_id == module_id::null)
                                          ? (module_count - 1)
                                          : (last_module_id));
                    ret_val.emplace_back(
                        make_separator_right(last_module_id).first);
                    return ret_val;
                  }
                }(),
                separator_string_cache, true);
          } else {
            i3bar_protocol::print_statusline(content_cache.first,
                                             content_string_cache, true);
          }

        } else if (!hide_current) {
          content_cache.first[cur_module_id].data.program = make_block::content(
              config.theme, content_cache.second[cur_module_id],
              (((module_id_to_active_index[cur_module_id]) % 2) != 0),
              config.general.custom_separators);

          if (config.general.custom_separators) {
            std::pair<std::pair<i3bar_data::block, module_id::type>,
                      std::pair<i3bar_data::block, module_id::type>>
                separators{make_separators(cur_module_id)};

            i3bar_protocol::print_statusline(
                content_cache.first[cur_module_id], cur_module_id,
                content_string_cache, separators.first.first,
                separators.first.second, separators.second.first,
                separators.second.second, separator_string_cache, true);
          } else {
            i3bar_protocol::print_statusline(content_cache.first[cur_module_id],
                                             cur_module_id,
                                             content_string_cache, true);
          }
        }
      }
    }
  } catch (const std::exception &error) {
    message_printing::error(error, true);
  } catch (...) {
    message_printing::error("unknown", true);
  }

  return EXIT_SUCCESS;
}
