#include "i3bar_protocol.hpp"
#include <chrono>
#include <thread>

int main() {
  using namespace i3bar_protocol;
  header h{1};
  output_header(h);
  output_infinite_array_start();
  block b1{.full_text{"hello world"}, .name{"hello world"}};
  block b2{.full_text{"bye"}, .name{"bye"}};
  std::vector<block> bs{b1, b2};
  while (true) {
    output_statusline(bs);
    std::this_thread::sleep_for(std::chrono::seconds{1});
  }
}
