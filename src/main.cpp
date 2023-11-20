#include <iostream>

#include "dyn_load_lib.hpp"
#include "dyn_load_module.hpp"
#include "i3bar_protocol.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "thread_comm.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "test module file path required\n";
    std::exit(1);
  } else {
    auto test_mod{dyn_load_module(argv[1])};
    auto tc_pair{thread_comm::make_thread_comm_pair<module_api::block>()};
    module_api api{std::move(tc_pair.first)};
    libconfigfile::map_node conf_in{};
    module_api::config_out conf_out{
        test_mod.second->init(std::move(api), std::move(conf_in))};
    test_mod.second->run();
    while (true) {
      tc_pair.second.wait();
      std::unique_ptr<module_api::block> new_block{tc_pair.second.get()};
      std::cout << new_block->full_text << '\n';
    }
  }
}
