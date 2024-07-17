#ifndef I3NEOSTATUS_PLUGIN_ID
#define I3NEOSTATUS_PLUGIN_ID

#include <cstddef>
#include <limits>
#include <string_view>

namespace i3neostatus {

namespace plugin_id {
using type = std::size_t;
static constexpr type null = std::numeric_limits<type>::max();
static constexpr type max = (null - 1);
type from_string(const std::string_view str);
} // namespace plugin_id

} // namespace i3neostatus
#endif
