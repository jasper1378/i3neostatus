#include "click_event_listener.hpp"

#include "i3bar_data.hpp"
#include "i3bar_protocol.hpp"
#include "module_handle.hpp"

#include <istream>
#include <utility>
#include <vector>

i3neostatus::click_event_listener::click_event_listener(
    std::vector<module_handle> *module_handles,
    std::istream *input_stream /*= &std::cin*/)
    : m_module_handles{module_handles}, m_input_stream{input_stream},
      m_thread{} {}

i3neostatus::click_event_listener::click_event_listener(
    click_event_listener &&other) noexcept
    : m_module_handles{other.m_module_handles},
      m_input_stream{other.m_input_stream},
      m_thread{std::move(other.m_thread)} {
  other.m_module_handles = nullptr;
  other.m_input_stream = nullptr;
}

i3neostatus::click_event_listener::~click_event_listener() {
  if (m_thread.joinable()) {
    m_thread.join();
  }
}

i3neostatus::click_event_listener &i3neostatus::click_event_listener::operator=(
    click_event_listener &&other) noexcept {
  if (this != &other) {
    m_module_handles = other.m_module_handles;
    m_input_stream = other.m_input_stream;
    m_thread = std::move(other.m_thread);

    other.m_module_handles = nullptr;
    other.m_input_stream = nullptr;
  }
  return *this;
}

void i3neostatus::click_event_listener::run() {
  m_thread = std::thread{[this]() -> void {
    i3bar_protocol::init_click_event(*m_input_stream);

    while (true) {
      i3bar_data::click_event click_event{
          i3bar_protocol::read_click_event(*m_input_stream)};
      (*m_module_handles)[click_event.id.instance].send_click_event(
          std::move(click_event.content));
    }
  }};
}
