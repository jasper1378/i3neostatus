#include <iostream>
#include <thread>

#include "dyn_lib_load.hpp"
#include "i3bar_protocol.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "thread_comm.hpp"

void producer(thread_comm::producer<int> &&p) {
  for (int i{0}; i < 10; ++i) {
    p.set_value(std::make_unique<int>(i));
    std::this_thread::sleep_for(std::chrono::seconds{1});
  }
  p.set_exception(std::make_exception_ptr(int{}));
}

void consumer(thread_comm::consumer<int> &&c) {
  for (;;) {
    c.wait();
    try {
      std::unique_ptr<int> p{c.get()};
      std::cerr << *p << '\n';
    } catch (...) {
      std::cerr << "PURPOSEFUL ERROR\n";
      return;
    }
  }
}

void foo(thread_comm::shared_state_state::type sss) {
  switch (sss) {
  case thread_comm::shared_state_state::NONE: {
    std::cerr << "NONE\n";
  } break;
  case thread_comm::shared_state_state::READ: {
    std::cerr << "READ\n";
  } break;
  case thread_comm::shared_state_state::UNREAD: {
    std::cerr << "UNREAD\n";
  } break;
  case thread_comm::shared_state_state::EXCEPTION: {
    std::cerr << "EXCEPTION\n";
  } break;
  }
}

int main(int argc, char *argv[]) {
  auto pair = thread_comm::make_thread_comm_pair<int>(
      &foo, (thread_comm::shared_state_state::READ |
             thread_comm::shared_state_state::UNREAD |
             thread_comm::shared_state_state::EXCEPTION));
  std::thread tp{producer, std::move(pair.first)};
  std::thread tc{consumer, std::move(pair.second)};
  tp.join();
  tc.join();
}
