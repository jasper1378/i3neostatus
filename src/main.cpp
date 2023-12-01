#include "dyn_load_lib.hpp"
#include "i3bar_protocol.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "module_handle.hpp"
#include "module_id.hpp"
#include "thread_comm.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "test module file path required\n";
    std::exit(1);
  } else {
    libconfigfile::map_node config{
        {"format",
         libconfigfile::make_node_ptr<libconfigfile::string_node>("%FT%TZ")}};
    module_handle test_mod{0, argv[1], std::move(config)};
    test_mod.run();
    for (std::size_t i = 0; i < 5; ++i) {
      test_mod.get_comm().wait();
      std::unique_ptr<module_api::block> new_block{test_mod.get_comm().get()};
      std::cout << new_block->full_text << '\n';
    }
  }
}
