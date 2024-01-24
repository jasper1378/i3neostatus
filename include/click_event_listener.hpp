#ifndef I3NEOSTATUS_CLICK_EVENT_LISTENER_HPP
#define I3NEOSTATUS_CLICK_EVENT_LISTENER_HPP

#include "module_handle.hpp"

#include <istream>
#include <vector>

namespace i3neostatus {

class click_event_listener {
private:
  std::vector<module_handle> *m_module_handles;
  std::istream *m_input_stream;
  std::thread m_thread;

public:
  click_event_listener(std::vector<module_handle> *module_handles,
                       std::istream *input_stream = &std::cin);

  click_event_listener(click_event_listener &&other) noexcept;

  click_event_listener(const click_event_listener &other) = delete;

  ~click_event_listener();

  click_event_listener &operator=(click_event_listener &&other) noexcept;

  click_event_listener &operator=(const click_event_listener &other) = delete;

public:
  void run();
};

} // namespace i3neostatus
#endif
