#include "block_state.hpp"
#include "click_event_listener.hpp"
#include "config_file.hpp"
#include "hide_block.hpp"
#include "i3bar_data.hpp"
#include "i3bar_protocol.hpp"
#include "make_block.hpp"
#include "message_printing.hpp"
#include "plugin_api.hpp"
#include "plugin_error.hpp"
#include "plugin_handle.hpp"
#include "plugin_id.hpp"

#include "bits-and-bytes/constexpr_hash_string.hpp"
#include "bits-and-bytes/unreachable_error.hpp"

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
    plugin_id::type id;
    class update_queue *update_queue;
    std::atomic<bool> is_buffered;

    explicit update_info(plugin_id::type id,
                         class update_queue *update_queue = nullptr,
                         std::atomic<bool> is_buffered = false)
        : id{id}, update_queue{update_queue}, is_buffered{is_buffered.load()} {}

    update_info(const update_info &other)
        : id{other.id}, update_queue{other.update_queue},
          is_buffered{other.is_buffered.load()} {}

    update_info(update_info &&other) noexcept
        : id{other.id}, update_queue{other.update_queue},
          is_buffered{other.is_buffered.load()} {
      other.id = plugin_id::null;
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

    update_info &operator=(update_info &&other) noexcept {
      if (this != &other) {
        id = other.id;
        update_queue = other.update_queue;
        is_buffered.store(other.is_buffered.load());

        other.id = plugin_id::null;
        other.update_queue = nullptr;
        other.is_buffered.store(false);
      }
      return *this;
    }
  };

private:
  std::size_t m_capacity;
  plugin_id::type *m_buffer;
  std::atomic<std::size_t> m_count;
  std::size_t m_write;
  std::mutex m_write_mtx;
  std::size_t m_read;

public:
  explicit update_queue(std::size_t capacity)
      : m_capacity{capacity}, m_buffer{new plugin_id::type[m_capacity]{}},
        m_count{0}, m_write{0}, m_read{0} {
    std::fill(m_buffer, m_buffer + m_capacity, plugin_id::null);
  }

  update_queue(update_queue &&other) noexcept
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

  update_queue &operator=(update_queue &&other) noexcept {
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
  void put(const plugin_id::type id) {
    {
      const std::lock_guard<std::mutex> lock_m_write_mtx{m_write_mtx};
      m_buffer[m_write] = id;
      m_write = inc_and_mod(m_write);
    }
    ++m_count;
    m_count.notify_all();
  }

  plugin_id::type get() {
    const plugin_id::type id{m_buffer[m_read]};
    m_read = inc_and_mod(m_read);
    --m_count;
    m_count.notify_all();
    return id;
  }

  const std::atomic<std::size_t> &count() const { return m_count; }

private:
  std::size_t inc_and_mod(const std::size_t i) const {
    return ((i + 1) % m_capacity);
  }
};

int main(int argc, char *argv[]) {
  try {
    const char *configuration_file_path{""};

    for (int cur_arg{1}; cur_arg < argc; ++cur_arg) {
      switch (bits_and_bytes::constexpr_hash_string::hash(argv[cur_arg])) {
      case bits_and_bytes::constexpr_hash_string::hash("-c"):
      case bits_and_bytes::constexpr_hash_string::hash("--config"): {
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
      case bits_and_bytes::constexpr_hash_string::hash("-h"):
      case bits_and_bytes::constexpr_hash_string::hash("--help"): {
        message_printing::help(argv[0]);
        return EXIT_SUCCESS;
      } break;
      case bits_and_bytes::constexpr_hash_string::hash("-v"):
      case bits_and_bytes::constexpr_hash_string::hash("--version"): {
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
    if (config.plugins.empty()) {
      message_printing::error("Umm... Are you forgetting something?", true);
    } else if (config.plugins.size() > plugin_id::max) {
      message_printing::error("Fool! That's too many plugins!", true);
    }
    const plugin_id::type plugin_count{config.plugins.size()};
    bool click_events_enabled{false};

    std::vector<plugin_handle> plugin_handles{};
    plugin_handles.reserve(plugin_count);
    std::vector<update_queue::update_info> plugin_updates{};
    plugin_updates.reserve(plugin_count);
    update_queue update_queue{plugin_count};

    std::pair<std::vector<struct i3bar_data::block>, std::vector<block_state>>
        content_cache{std::piecewise_construct, std::forward_as_tuple(),
                      std::forward_as_tuple(plugin_count, block_state::idle)};
    content_cache.first.reserve(plugin_count);
    std::vector<plugin_id::type> plugin_id_to_active_index(plugin_count,
                                                           plugin_id::null);
    std::vector<plugin_id::type> active_index_to_plugin_id(plugin_count,
                                                           plugin_id::null);

    std::vector<std::string> content_string_cache(plugin_count);
    std::vector<std::string> separator_string_cache(
        (config.general.custom_separators) ? (plugin_count + 1) : (0));

    const auto plugin_callback{
        [](void *userdata,
           [[maybe_unused]] plugin_handle::state_change_type state) -> void {
          update_queue::update_info *plugin_update{
              static_cast<update_queue::update_info *>(userdata)};
          if (!plugin_update->is_buffered.load()) {
            plugin_update->is_buffered.store(true);
            plugin_update->update_queue->put(plugin_update->id);
          }
        }};

    for (plugin_id::type cur_plugin_id{0}; cur_plugin_id < plugin_count;
         ++cur_plugin_id) {
      plugin_updates.emplace_back(cur_plugin_id, &update_queue, false);
      plugin_handles.emplace_back(
          cur_plugin_id, std::move(config.plugins[cur_plugin_id].file_path),
          std::move(config.plugins[cur_plugin_id].config),
          plugin_handle::state_change_callback{plugin_callback,
                                               &plugin_updates.back()});
      click_events_enabled =
          (click_events_enabled ||
           plugin_handles[cur_plugin_id].get_click_events_enabled());
      plugin_handles.back().run();
      content_cache.first.emplace_back(i3bar_data::block{
          .id{.name{plugin_handles.back().get_name()},
              .instance{cur_plugin_id}},
          .data{.plugin{
              hide_block::set<struct i3bar_data::block::data::plugin>()}}});
    }

    click_event_listener click_event_listener{&plugin_handles, &std::cin};
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
        const plugin_id::type cur_plugin_id{update_queue.get()};
        plugin_updates[cur_plugin_id].is_buffered.store(false);

        const bool hide_previous{
            hide_block::get(content_cache.first[cur_plugin_id])};
        std::variant<plugin_api::block, std::exception_ptr> content_plugin{
            plugin_handles[cur_plugin_id].get_comm().get()};

        switch (content_plugin.index()) {
        case 0: {
          std::tie(content_cache.first[cur_plugin_id].data.plugin,
                   content_cache.second[cur_plugin_id]) =
              std::get<0>(std::move(content_plugin));
        } break;
        case 1: {
          try {
            std::rethrow_exception(std::get<1>(std::move(content_plugin)));
          } catch (const std::exception &exception) {
            content_cache.first[cur_plugin_id].data.plugin = {
                .full_text{plugin_error{
                    cur_plugin_id, plugin_handles[cur_plugin_id].get_name(),
                    plugin_handles[cur_plugin_id].get_file_path(),
                    exception.what()}
                               .what()},
                .short_text{std::nullopt},
                .min_width{std::nullopt},
                .align{std::nullopt},
                .urgent{true},
                .markup{i3bar_data::types::markup::none}};
            content_cache.second[cur_plugin_id] = block_state::error;
          }
        } break;
        default: {
          throw bits_and_bytes::unreachable_error{};
        } break;
        }

        const bool hide_current{
            hide_block::get(content_cache.first[cur_plugin_id])};

        const auto make_separator_left{
            [&config, &content_cache, &plugin_id_to_active_index,
             &active_index_to_plugin_id](const plugin_id::type cur_plugin_id)
                -> std::pair<i3bar_data::block, plugin_id::type> {
              if (cur_plugin_id == plugin_id::null) {
                return {i3bar_data::block{.data{.plugin{hide_block::set<
                            struct i3bar_data::block::data::plugin>()}}},
                        plugin_id::null};
              }
              const plugin_id::type left_plugin_id{
                  ((plugin_id_to_active_index[cur_plugin_id] != 0)
                       ? (active_index_to_plugin_id
                              [plugin_id_to_active_index[cur_plugin_id] - 1])
                       : (plugin_id::null))};
              return {
                  make_block::separator(
                      config.theme,
                      ((left_plugin_id != plugin_id::null)
                           ? (&content_cache.first[left_plugin_id]
                                   .data.program.theme)
                           : (nullptr)),
                      &content_cache.first[cur_plugin_id].data.program.theme),
                  cur_plugin_id};
            }};
        const auto make_separator_right{
            [&config, plugin_count, &content_cache, &plugin_id_to_active_index,
             &active_index_to_plugin_id](const plugin_id::type cur_plugin_id)
                -> std::pair<i3bar_data::block, plugin_id::type> {
              if (cur_plugin_id == plugin_id::null) {
                return {i3bar_data::block{.data{.plugin{hide_block::set<
                            struct i3bar_data::block::data::plugin>()}}},
                        plugin_id::null};
              }
              const plugin_id::type right_plugin_id{
                  ((((plugin_id_to_active_index[cur_plugin_id] + 1 <
                      plugin_count)) &&
                    (active_index_to_plugin_id
                         [plugin_id_to_active_index[cur_plugin_id] + 1] !=
                     plugin_id::null))
                       ? (active_index_to_plugin_id
                              [plugin_id_to_active_index[cur_plugin_id] + 1])
                       : (plugin_id::null))};
              return {
                  make_block::separator(
                      config.theme,
                      &content_cache.first[cur_plugin_id].data.program.theme,
                      ((right_plugin_id != plugin_id::null)
                           ? (&content_cache.first[right_plugin_id]
                                   .data.program.theme)
                           : (nullptr))),
                  ((right_plugin_id != plugin_id::null) ? (right_plugin_id)
                                                        : (plugin_count))};
            }};
        const auto make_separators{
            [&make_separator_left,
             &make_separator_right](const plugin_id::type cur_plugin_id)
                -> std::pair<decltype(make_separator_left(cur_plugin_id)),
                             decltype(make_separator_right(cur_plugin_id))> {
              return {make_separator_left(cur_plugin_id),
                      make_separator_right(cur_plugin_id)};
            }};

        if (hide_previous != hide_current) {
          if (hide_current) {
            for (plugin_id::type i{plugin_id_to_active_index[cur_plugin_id]};
                 i < (plugin_count - 1); ++i) {
              active_index_to_plugin_id[i] = active_index_to_plugin_id[i + 1];
            }
            active_index_to_plugin_id.back() = plugin_id::null;
            for (plugin_id::type i{cur_plugin_id + 1}; i < plugin_count; ++i) {
              if (plugin_id_to_active_index[i] != plugin_id::null) {
                --plugin_id_to_active_index[i];
              }
            }
            plugin_id_to_active_index[cur_plugin_id] = plugin_id::null;
          } else {
            for (plugin_id::type i{0};; ++i) {
              if ((active_index_to_plugin_id[i] > cur_plugin_id) ||
                  (active_index_to_plugin_id[i] == plugin_id::null)) {
                for (plugin_id::type j{plugin_count}; (j--) > (i + 1);) {
                  active_index_to_plugin_id[j] =
                      active_index_to_plugin_id[j - 1];
                }
                active_index_to_plugin_id[i] = cur_plugin_id;
                for (plugin_id::type j{cur_plugin_id + 1}; j < plugin_count;
                     ++j) {
                  if (plugin_id_to_active_index[j] != plugin_id::null) {
                    ++plugin_id_to_active_index[j];
                  }
                }
                plugin_id_to_active_index[cur_plugin_id] = i;
                break;
              }
            }
          }

          for (plugin_id::type i{0}; i < plugin_count; ++i) {
            if (!hide_block::get(content_cache.first[i])) {
              content_cache.first[i].data.program = make_block::content(
                  config.theme, content_cache.second[i],
                  (((plugin_id_to_active_index[i]) % 2) != 0),
                  config.general.custom_separators);
            }
          }

          if (config.general.custom_separators) {
            i3bar_protocol::print_statusline(
                content_cache.first, content_string_cache,
                [plugin_count, &plugin_id_to_active_index,
                 &active_index_to_plugin_id, &make_separator_left,
                 &make_separator_right]() -> std::vector<i3bar_data::block> {
                  if (active_index_to_plugin_id.front() == plugin_id::null) {
                    return std::vector<i3bar_data::block>{
                        (plugin_count + 1),
                        i3bar_data::block{.data{.plugin{hide_block::set<
                            struct i3bar_data::block::data::plugin>()}}}};
                  } else {
                    std::vector<i3bar_data::block> ret_val;
                    ret_val.reserve(plugin_count + 1);
                    plugin_id::type last_plugin_id{plugin_id::null};
                    for (plugin_id::type i{0}; i < plugin_count; ++i) {
                      if ((active_index_to_plugin_id[i] == plugin_id::null) &&
                          (last_plugin_id == plugin_id::null)) {
                        last_plugin_id = active_index_to_plugin_id[i - 1];
                      }
                      ret_val.emplace_back(
                          make_separator_left(
                              (plugin_id_to_active_index[i] != plugin_id::null)
                                  ? (i)
                                  : (plugin_id::null))
                              .first);
                    }
                    last_plugin_id = ((last_plugin_id == plugin_id::null)
                                          ? (plugin_count - 1)
                                          : (last_plugin_id));
                    ret_val.emplace_back(
                        make_separator_right(last_plugin_id).first);
                    return ret_val;
                  }
                }(),
                separator_string_cache, true);
          } else {
            i3bar_protocol::print_statusline(content_cache.first,
                                             content_string_cache, true);
          }

        } else if (!hide_current) {
          content_cache.first[cur_plugin_id].data.program = make_block::content(
              config.theme, content_cache.second[cur_plugin_id],
              (((plugin_id_to_active_index[cur_plugin_id]) % 2) != 0),
              config.general.custom_separators);

          if (config.general.custom_separators) {
            const std::pair<std::pair<i3bar_data::block, plugin_id::type>,
                            std::pair<i3bar_data::block, plugin_id::type>>
                separators{make_separators(cur_plugin_id)};

            i3bar_protocol::print_statusline(
                content_cache.first[cur_plugin_id], cur_plugin_id,
                content_string_cache, separators.first.first,
                separators.first.second, separators.second.first,
                separators.second.second, separator_string_cache, true);
          } else {
            i3bar_protocol::print_statusline(content_cache.first[cur_plugin_id],
                                             cur_plugin_id,
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
