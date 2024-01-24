#include "click_event_listener.hpp"
#include "config_file.hpp"
#include "i3bar_data.hpp"
#include "i3bar_protocol.hpp"
#include "misc.hpp"
#include "module_error.hpp"
#include "module_handle.hpp"
#include "module_id.hpp"
#include "program_constants.hpp"

#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>

class update_queue {
public:
  struct update_info {
    i3neostatus::module_id::type id;
    class update_queue *update_queue;
    std::atomic<bool> is_buffered;

    update_info(i3neostatus::module_id::type id,
                class update_queue *update_queue = nullptr,
                std::atomic<bool> is_buffered = false)
        : id{id}, update_queue{update_queue}, is_buffered{is_buffered.load()} {}

    update_info(const update_info &other)
        : id{other.id}, update_queue{other.update_queue},
          is_buffered{other.is_buffered.load()} {}

    update_info(update_info &&other)
        : id{other.id}, update_queue{other.update_queue},
          is_buffered{other.is_buffered.load()} {
      other.id = i3neostatus::module_id::null;
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

        other.id = i3neostatus::module_id::null;
        other.update_queue = nullptr;
        other.is_buffered.store(false);
      }
      return *this;
    }
  };

private:
  std::size_t m_capacity;
  i3neostatus::module_id::type *m_buffer;
  std::atomic<std::size_t> m_count;
  std::size_t m_write;
  std::mutex m_write_mtx;
  std::size_t m_read;

public:
  update_queue(std::size_t capacity)
      : m_capacity{capacity},
        m_buffer{new i3neostatus::module_id::type[m_capacity]{}}, m_count{0},
        m_write{0}, m_read{0} {
    std::fill(m_buffer, m_buffer + m_capacity, i3neostatus::module_id::null);
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
  void put(const i3neostatus::module_id::type id) {
    {
      std::lock_guard<std::mutex> lock_m_write_mtx{m_write_mtx};
      m_buffer[m_write] = id;
      m_write = inc_and_mod(m_write);
    }
    ++m_count;
    m_count.notify_all();
  }

  i3neostatus::module_id::type get() {
    i3neostatus::module_id::type id{m_buffer[m_read]};
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

void print_program_info(std::ostream &output_stream = std::cout) {
  output_stream << i3neostatus::program_constants::g_k_name << ' '
                << i3neostatus::program_constants::g_k_version << " © "
                << i3neostatus::program_constants::g_k_year << ' '
                << i3neostatus::program_constants::g_k_authors << '\n';
}

void print_help(
    const std::string_view argv_0 = i3neostatus::program_constants::g_k_name,
    std::ostream &output_stream = std::cout) {
  print_program_info(output_stream);
  output_stream << "Syntax: " << argv_0 << " [-c <configfile>] [-h] [-v]\n";
}

void print_version(std::ostream &output_stream = std::cout) {
  print_program_info(output_stream);
}

void print_error(const std::string_view error, bool exit = false,
                 std::ostream &output_stream = std::cerr) {
  output_stream << "Error: " << error << '\n';
  if (exit) {
    std::exit(EXIT_FAILURE);
  }
}

void print_error(const std::exception &exception, bool exit = false,
                 std::ostream &output_stream = std::cerr) {
  print_error(exception.what(), exit, output_stream);
}

int main(int argc, char *argv[]) {
  try {
    const char *configuration_file_path{""};

    for (int cur_arg{1}; cur_arg < argc; ++cur_arg) {
      switch (i3neostatus::misc::constexpr_hash_string::hash(argv[cur_arg])) {
      case i3neostatus::misc::constexpr_hash_string::hash("-c"):
      case i3neostatus::misc::constexpr_hash_string::hash("--config"): {
        if (((cur_arg + 1) < argc) && (*argv[cur_arg + 1] != '-')) {
          if (*configuration_file_path == '\0') {
            configuration_file_path = argv[++cur_arg];
          } else {
            print_error((std::string{'"'} + argv[cur_arg] +
                         "\" option has already been specified\n"),
                        true);
          }
        } else {
          print_error((std::string{'"'} + argv[cur_arg] +
                       "\" option requires an argument"),
                      true);
        }
      } break;
      case i3neostatus::misc::constexpr_hash_string::hash("-h"):
      case i3neostatus::misc::constexpr_hash_string::hash("--help"): {
        print_help(argv[0]);
        return 0;
      } break;
      case i3neostatus::misc::constexpr_hash_string::hash("-v"):
      case i3neostatus::misc::constexpr_hash_string::hash("--version"): {
        print_version();
        return 0;
      } break;
      default: {
        print_error(
            (std::string{'"'} + argv[cur_arg] + "\" option is unrecognized\n"),
            true);
      } break;
      }
    }

    i3neostatus::config_file::parsed config{
        ((*configuration_file_path == '\0')
             ? (i3neostatus::config_file::read())
             : (i3neostatus::config_file::read(configuration_file_path)))};
    if (config.modules.size() > i3neostatus::module_id::max) {
      print_error(("too many modules! max is " +
                   std::to_string(i3neostatus::module_id::max) + ')'),
                  true);
    }
    const i3neostatus::module_id::type module_count{config.modules.size()};

    std::vector<i3neostatus::module_handle> module_handles{};
    module_handles.reserve(module_count);

    std::vector<update_queue::update_info> module_updates{};
    module_updates.reserve(module_count);

    update_queue update_queue{module_count};

    const auto module_callback{
        [](void *userdata,
           [[maybe_unused]] i3neostatus::module_handle::state_change_type state)
            -> void {
          update_queue::update_info *module_update{
              static_cast<update_queue::update_info *>(userdata)};
          if (module_update->is_buffered.load() == false) {
            module_update->is_buffered.store(true);
            module_update->update_queue->put(module_update->id);
          }
        }};

    bool any_click_events_enabled{false};
    for (i3neostatus::module_id::type i = 0; i < module_count; ++i) {
      module_updates.emplace_back(i, &update_queue, false);
      module_handles.emplace_back(
          i, std::move(config.modules[i].file_path),
          std::move(config.modules[i].config),
          i3neostatus::module_handle::state_change_callback{
              module_callback, &module_updates.back()});
      any_click_events_enabled = any_click_events_enabled ||
                                 module_handles[i].get_click_events_enabled();
      module_handles.back().run();
    }

    i3neostatus::click_event_listener click_event_listener{&module_handles,
                                                           &std::cin};
    if (any_click_events_enabled) {
      click_event_listener.run();
    }

    i3neostatus::i3bar_protocol::print_header(
        {1, SIGSTOP, SIGCONT, any_click_events_enabled});
    i3neostatus::i3bar_protocol::init_statusline();
    std::vector<std::string> i3bar_cache(module_count);

    while (true) {
      update_queue.count().wait(0);
      for (std::size_t queued_updates{update_queue.count().load()},
           cur_queued_update{};
           cur_queued_update < queued_updates; ++cur_queued_update) {
        i3neostatus::module_id::type cur_module_id{update_queue.get()};

        module_updates[cur_module_id].is_buffered.store(false);
        std::variant<i3neostatus::module_api::block, std::exception_ptr>
            block_content{module_handles[cur_module_id].get_comm().get()};

        auto make_updated_block{
            [&module_handles, &cur_module_id](
                i3neostatus::i3bar_data::block::struct_content &&block_content)
                -> std::pair<i3neostatus::i3bar_data::block,
                             i3neostatus::module_id::type> {
              return std::pair<i3neostatus::i3bar_data::block,
                               i3neostatus::module_id::type>{
                  {i3neostatus::i3bar_data::block::struct_id{
                       module_handles[cur_module_id].get_name(),
                       module_handles[cur_module_id].get_id()},
                   {std::move(block_content)}},
                  cur_module_id};
            }};

        switch (block_content.index()) {
        case 0: {
          i3neostatus::i3bar_protocol::print_statusline(
              make_updated_block(std::get<0>(std::move(block_content))),
              i3bar_cache, true);
        } break;
        case 1: {
          try {
            std::rethrow_exception(std::get<1>(std::move(block_content)));
          } catch (const std::exception &exception) {
            i3neostatus::i3bar_protocol::print_statusline(
                make_updated_block(
                    i3neostatus::i3bar_data::block::struct_content{
                        .full_text{i3neostatus::module_error{
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
