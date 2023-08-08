#include "i3bar_protocol.hpp"

int main() {
  using namespace i3bar_protocol;
  header h{.version{1}};
  output_header(h);
  output_infinite_array_start();
  block b{.full_text{"hello world"}};
  output_statusline(std::vector<block>({b}));
}
