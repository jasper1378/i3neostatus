#include "thread_comm.hpp"

#include <cassert>
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

#undef assert
#define assert(B)                                                              \
  {                                                                            \
    if (!(B)) {                                                                \
      std::cerr << "assertion (" << #B << ") failed\n";                        \
      std::exit(1);                                                            \
    }                                                                          \
  }

using t_data = int;

void producer_test(thread_comm::producer<t_data> &&p) {
  for (int i{0}; i < 5;
       ++i, std::this_thread::sleep_for(std::chrono::seconds{1})) {
    p.set_value(std::make_unique<t_data>(i));
  }
  p.set_exception(std::make_exception_ptr(std::runtime_error{"bye!"}));
}

void consumer_test(thread_comm::consumer<t_data> &&c) {
  while (true) {
    c.wait();
    try {
      std::unique_ptr<t_data> v{c.get()};
      std::cerr << *v << '\n';
    } catch (const std::exception &e) {
      std::cerr << "Ex: " << e.what() << '\n';
      std::exit(1);
    } catch (...) {
      std::cerr << "Ex: UNK\n";
      std::exit(1);
    }
  }
}

int main() {
  auto p{thread_comm::make_thread_comm_pair<t_data>()};
  std::thread tp{producer_test, std::move(p.first)};
  std::thread tc{consumer_test, std::move(p.second)};
  while (true) {
    std::this_thread::sleep_for(std::chrono::years{1});
  }
}
