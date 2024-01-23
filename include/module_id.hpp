#ifndef I3NEOSTATUS_MODULE_ID
#define I3NEOSTATUS_MODULE_ID

#include <cstddef>
#include <limits>
#include <string_view>

namespace module_id {
using type = std::size_t;
static constexpr type max = std::numeric_limits<type>::max() - 1;
static constexpr type null = std::numeric_limits<type>::max();
type from_string(const std::string_view str);
} // namespace module_id

#endif
