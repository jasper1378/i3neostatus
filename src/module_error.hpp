#ifndef I3NEOSTATUS_MODULE_ERROR_HPP
#define I3NEOSTATUS_MODULE_ERROR_HPP

#include "module_id.hpp"

#include <exception>
#include <stdexcept>
#include <string>

namespace i3neostatus {

class module_error : public std::runtime_error {
private:
  using base_t = std::runtime_error;

public:
  module_error(const module_id::type id, const std::string &name,
               const std::string &file_path, const std::string &what_arg);
  module_error(const module_id::type id, const char *name,
               const char *file_path, const char *what_arg);
  module_error(const module_error &other);

public:
  virtual ~module_error() override;

public:
  module_error &operator=(const module_error &other);

public:
  virtual const char *what() const noexcept override;
};

} // namespace i3neostatus
#endif
