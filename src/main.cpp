#include "config_file.hpp"
#include "i3bar_protocol.hpp"
#include "module_error.hpp"
#include "module_handle.hpp"
#include "module_id.hpp"

#include <algorithm>
#include <atomic>
#include <csignal>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>

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

    i3bar_protocol::print_header(
        {1, SIGSTOP, SIGCONT, any_click_events_enabled}, std::cout);
    i3bar_protocol::init_statusline(std::cout);
    std::vector<std::string> i3bar_cache(module_count);

    while (true) {
      update_queue.count().wait(0);
      for (std::size_t queued_updates{update_queue.count().load()},
           cur_queued_update{};
           cur_queued_update < queued_updates; ++cur_queued_update) {
        module_id::type cur_module_id{update_queue.get()};

        module_updates[cur_module_id].is_buffered.store(false);
        std::variant<module_api::block, std::exception_ptr> block_content{
            module_handles[cur_module_id].get_comm().get()};

        switch (block_content.index()) {
        case 0: {
          std::pair<i3bar_protocol::block, module_id::type> updated_block{
              {i3bar_protocol::block::struct_id{
                   module_handles[cur_module_id].get_name(),
                   module_handles[cur_module_id].get_id()},
               std::get<0>(std::move(block_content))},
              cur_module_id};
          i3bar_protocol::print_statusline(updated_block, i3bar_cache, true,
                                           std::cout);
        } break;
        case 1: {
          try {
            std::rethrow_exception(std::get<1>(std::move(block_content)));
          } catch (const std::exception &ex) {
            throw module_error{module_handles[cur_module_id].get_id(),
                               module_handles[cur_module_id].get_name(),
                               module_handles[cur_module_id].get_file_path(),
                               ex.what()};
          }
        } break;
        }
      }
    }
  }
}
