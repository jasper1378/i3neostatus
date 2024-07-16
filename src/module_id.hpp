#ifndef I3NEOSTATUS_MODULE_ID
#define I3NEOSTATUS_MODULE_ID

#include <cstddef>
#include <limits>
#include <string_view>

namespace i3neostatus {

namespace module_id {
using type = std::size_t;
static constexpr type null = std::numeric_limits<type>::max();
static constexpr type max = (null - 1);
type from_string(const std::string_view str);
} // namespace module_id

} // namespace i3neostatus
#endif
