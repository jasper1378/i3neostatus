#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include "libconfigfile.hpp"

#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace config_file {
class error : public std::runtime_error {
private:
  using base_t = std::runtime_error;

private:
  static constexpr char m_k_separator_char{':'};
  static constexpr char m_k_whitespace_char{' '};

public:
  error(const std::string &message, const std::string &file_path = "",
        const long long pos_line = -1, const long long pos_char = -1);
  error(const char *what_arg, const char *file_path = "",
        const long long pos_line = -1, const long long pos_char = -1);
  error(const error &other);

public:
  virtual ~error() override;

public:
  error &operator=(const error &other);

public:
  virtual const char *what() const noexcept override;
};

struct parsed {
  struct general {
    int dummy_TODO;
  };

  struct module {
    std::filesystem::path file_path;
    libconfigfile::map_node config;
  };

  general general;
  std::vector<module> modules;
};

parsed read(const char *file_path);
parsed read(const std::string &file_path);
parsed read(const std::filesystem::path &file_path);
parsed read();

namespace impl {
std::string resolve_path(const std::string &file_path);
std::string resolve_path(std::string &&file_path);
parsed read(const std::string &file_path);
libconfigfile::node_ptr<libconfigfile::map_node>
libcf_parse_file_wrapper(const std::string &file_path);
}; // namespace impl

}; // namespace config_file

#endif
