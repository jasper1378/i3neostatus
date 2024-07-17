#ifndef I3NEOSTATUS_PLUGIN_ERROR_HPP
#define I3NEOSTATUS_PLUGIN_ERROR_HPP

#include "plugin_id.hpp"

#include <exception>
#include <stdexcept>
#include <string>

namespace i3neostatus {

class plugin_error : public std::runtime_error {
private:
  using base_t = std::runtime_error;

public:
  plugin_error(const plugin_id::type id, const std::string &name,
               const std::string &file_path, const std::string &what_arg);
  plugin_error(const plugin_id::type id, const char *name,
               const char *file_path, const char *what_arg);
  plugin_error(const plugin_error &other);

public:
  virtual ~plugin_error() override;

public:
  plugin_error &operator=(const plugin_error &other);

public:
  virtual const char *what() const noexcept override;
};

} // namespace i3neostatus
#endif
