#ifndef I3NEOSTATUS_PLUGIN_ERROR_HPP
#define I3NEOSTATUS_PLUGIN_ERROR_HPP

#include "plugin_id.hpp"

#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <variant>

namespace i3neostatus {

class plugin_error : public std::runtime_error {
private:
  using base_t = std::runtime_error;

public:
  plugin_error(const plugin_id::type id, const std::filesystem::path &path,
               const std::string &what_arg);
  plugin_error(const plugin_id::type id, const std::string &name,
               const std::string &what_arg);
  plugin_error(
      const plugin_id::type id,
      const std::variant<std::filesystem::path, std::string> &path_or_name,
      const std::string &what_arg);
  plugin_error(const plugin_error &other);

public:
  virtual ~plugin_error() override;

public:
  plugin_error &operator=(const plugin_error &other);

public:
  virtual const char *what() const noexcept override;

private:
  static base_t do_ctor(const plugin_id::type id,
                        const std::filesystem::path &path,
                        const std::string &what_arg);
  static base_t do_ctor(const plugin_id::type id, const std::string &name,
                        const std::string &what_arg);
};

} // namespace i3neostatus
#endif
