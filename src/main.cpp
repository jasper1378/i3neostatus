#include "config_file.hpp"
#include "dyn_load_lib.hpp"
#include "i3bar_protocol.hpp"
#include "misc.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "module_error.hpp"
#include "module_handle.hpp"
#include "module_id.hpp"
#include "thread_comm.hpp"

#include <iostream>
#include <vector>

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
    std::vector<module_handle> modules{};
    modules.reserve(config.modules.size());

    for (module_id::type i = 0; i < config.modules.size(); ++i) {
      // modules.emplace_back(i, config.modules[i].file_path,
      //                      std::move(config.modules[i].config));
      // modules[i].run();
      // for (std::size_t i = 0; i < 5; ++i) {
      //   handles[i].get_comm().wait();
      //   std::unique_ptr<module_api::block> new_block{
      //       handles[i].get_comm().get()};
      //   std::cout << new_block->full_text << '\n';
      // }
    }
  }
}
