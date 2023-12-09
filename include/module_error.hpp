#ifndef MODULE_ERROR_HPP
#define MODULE_ERROR_HPP

#include "module_id.hpp"

#include <exception>
#include <stdexcept>
#include <string>

namespace module_error {
class in : public std::runtime_error {
private:
  using base_t = std::runtime_error;

public:
  in(const module_id_t id, const std::string &name,
     const std::string &file_path, const std::string &what_arg);
  in(const module_id_t id, const char *name, const char *file_path,
     const char *what_arg);
  in(const in &other);

public:
  virtual ~in() override;

public:
  in &operator=(const in &other);

public:
  virtual const char *what() const noexcept override;
};

class out : public std::runtime_error {
private:
  using base_t = std::runtime_error;

public:
  out(const module_id_t id, const std::string &name,
      const std::string &file_path, const std::string &what_arg);
  out(const module_id_t id, const char *name, const char *file_path,
      const char *what_arg);
  out(const out &other);

public:
  virtual ~out() override;

public:
  out &operator=(const out &other);

public:
  virtual const char *what() const noexcept override;
};
} // namespace module_error

#endif
